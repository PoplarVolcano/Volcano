#include "volpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/attrdefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"

#include "FileWatch.hpp"

#include "Volcano/Core/Application.h"
#include "Volcano/Core/Timer.h"
#include "Volcano/Core/Buffer.h"
#include "Volcano/Core/FileSystem.h"

namespace Volcano {

	// C#��float�ռ��� + ������->System.Single����ת�������Զ���������ơ�Float����
	// �Զ���C#����������
	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Single",   ScriptFieldType::Float   },
		{ "System.Double",   ScriptFieldType::Double  },
		{ "System.Boolean",  ScriptFieldType::Bool    },
		{ "System.Char",     ScriptFieldType::Char    },
		{ "System.Int16",    ScriptFieldType::Short   },
		{ "System.Int32",    ScriptFieldType::Int     },
		{ "System.Int64",    ScriptFieldType::Long    },
		{ "System.Byte",     ScriptFieldType::Byte    },
		{ "System.UInt16",   ScriptFieldType::UShort  },
		{ "System.UInt32",   ScriptFieldType::UInt    },
		{ "System.UInt64",   ScriptFieldType::ULong   },
													 
		{ "Volcano.Vector2", ScriptFieldType::Vector2 },
		{ "Volcano.Vector3", ScriptFieldType::Vector3 },
		{ "Volcano.Vector4", ScriptFieldType::Vector4 },
													 
		{ "Volcano.Entity",  ScriptFieldType::Entity  },
	};

	namespace Utils {

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			ScopedBuffer fileData = FileSystem::ReadFileBinary(assemblyPath);

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), fileData.Size(), 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				return nullptr;
			}

			// ����debug
			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");
				if (std::filesystem::exists(pdbPath))
				{
					ScopedBuffer pdbFileData = FileSystem::ReadFileBinary(pdbPath);
					mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), pdbFileData.Size());
					std::cout << "Loaded PDB " << pdbPath.generic_string() << std::endl;
				}
			}
			
			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			return assembly;
		}

		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				VOL_CORE_TRACE("{}::{}", nameSpace, name);
			}
		}

		// C#������תC++�����ͱ��
		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = s_ScriptFieldTypeMap.find(typeName);
			if (it == s_ScriptFieldTypeMap.end())
			{
				VOL_CORE_ERROR("Unknown type: {}", typeName);
				return ScriptFieldType::None;
			}

			return it->second;
		}
	}

	struct ScriptEngineData
	{
		// Mono����
		MonoDomain* RootDomain = nullptr;
		// Mono App��
		MonoDomain* AppDomain = nullptr;

		// Core����
		MonoAssembly* CoreAssembly = nullptr;
		// Core��������
		MonoImage* CoreAssemblyImage = nullptr;

		// App����
		MonoAssembly* AppAssembly = nullptr;
		// App��������
		MonoImage* AppAssemblyImage = nullptr;

		// Core����dll�ļ�·��
		std::filesystem::path CoreAssemblyFilepath;
		// App����dll�ļ�·��
		std::filesystem::path AppAssemblyFilepath;

		// һ��ʵ��ID��Ӧ�������ֶ�
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		// C#Entity��ʵ��
		ScriptClass EntityClass;

		// C#�е�Entity���࣬��Player
		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		// ID��Ӧ��C#�е�Entity����ʵ������Player
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		// �ļ������
		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
		bool AssemblyReloadPending = false;

		bool EnableDebugging = true;

		// Runtime
		Scene* SceneContext = nullptr;
	};
	static ScriptEngineData* s_ScriptEngineData = nullptr;

	// �ļ��������¼������״̬Ϊ�޸ģ���� �ؼ���C#���򼯷��� ����Application���̶߳���
	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type)
	{
		if (!s_ScriptEngineData->AssemblyReloadPending && change_type == filewatch::Event::modified)
		{
			s_ScriptEngineData->AssemblyReloadPending = true;

			Application::Get().SubmitToMainThread([]()
				{
					s_ScriptEngineData->AppAssemblyFileWatcher.reset();
					ScriptEngine::ReloadAssembly();
				});
		}
	}

	void ScriptEngine::Init()
	{
		s_ScriptEngineData = new ScriptEngineData();

		// 1 ��ʼ��mono
		InitMono();

		// ����ڲ�����
		ScriptGlue::RegisterFunctions();

		// 2.����c#����
		bool status = LoadAssembly("Resources/Scripts/Volcano-ScriptCore.dll");
		if (!status)
		{
			VOL_CORE_ERROR("[ScriptEngine] Could not load Volcano-ScriptCore assembly.");
			return;
		}
		status = LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll");
		if (!status)
		{
			VOL_CORE_ERROR("[ScriptEngine] Could not load app assembly.");
			return;
		}
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		// ��������Entity����-Ϊ���ڵ���OnCreate����֮ǰ��UUID����C#Entity�Ĺ��캯��
		s_ScriptEngineData->EntityClass = ScriptClass("Volcano", "Entity", true);
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_ScriptEngineData;
	}


	void ScriptEngine::InitMono()
	{
		// Let Mono know where the .NET libraries are located.
		// û��MONO_PATH����������Ĭ������ڵ�ǰ����Ŀ¼(VolcanoNut)��·�� 
		mono_set_assemblies_path("../Volcano/vendor/mono/lib");

		
		if (s_ScriptEngineData->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}
		
		// ��������
		MonoDomain* rootDomain = mono_jit_init("VolcanoJITRuntime");
		if (rootDomain == nullptr)
		{
			// Maybe log some error here
			return;
		}
		// �洢root domainָ��
		s_ScriptEngineData->RootDomain = rootDomain;

		if (s_ScriptEngineData->EnableDebugging)
			mono_debug_domain_create(s_ScriptEngineData->RootDomain);

		mono_thread_set_main(mono_thread_current());

	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_ScriptEngineData->AppDomain);
		s_ScriptEngineData->AppDomain = nullptr;

		mono_jit_cleanup(s_ScriptEngineData->RootDomain);
		s_ScriptEngineData->RootDomain = nullptr;
	}

	// ��ȡC# dll�ļ�
	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_ScriptEngineData->AppDomain = mono_domain_create_appdomain((char*)"VolcanoScriptRuntime", nullptr);
		mono_domain_set(s_ScriptEngineData->AppDomain, true);

		s_ScriptEngineData->CoreAssemblyFilepath = filepath;
		// ����c#��Ŀ������dll
		s_ScriptEngineData->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_ScriptEngineData->EnableDebugging);
		if (s_ScriptEngineData->CoreAssembly == nullptr)
			return false;
		// �õ�MonoImage����
		s_ScriptEngineData->CoreAssemblyImage = mono_assembly_get_image(s_ScriptEngineData->CoreAssembly);
		//Utils::PrintAssemblyTypes(s_ScriptEngineData->CoreAssembly);// ��ӡdll�Ļ�����Ϣ
		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		s_ScriptEngineData->AppAssemblyFilepath = filepath;
		// Move this maybe
		s_ScriptEngineData->AppAssembly = Utils::LoadMonoAssembly(filepath, s_ScriptEngineData->EnableDebugging);
		auto assemb = s_ScriptEngineData->AppAssembly;
		s_ScriptEngineData->AppAssemblyImage = mono_assembly_get_image(s_ScriptEngineData->AppAssembly);
		if (s_ScriptEngineData->AppAssembly == nullptr)
			return false;
		//Utils::PrintAssemblyTypes(s_ScriptEngineData->AppAssembly);

		// �����ļ�������
		s_ScriptEngineData->AppAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(filepath.string(), OnAppAssemblyFileSystemEvent);
		s_ScriptEngineData->AssemblyReloadPending = false;

		return true;
	}

	void ScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_ScriptEngineData->AppDomain);

		LoadAssembly(s_ScriptEngineData->CoreAssemblyFilepath);
		LoadAppAssembly(s_ScriptEngineData->AppAssemblyFilepath);
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		// Retrieve and instantiate class
		s_ScriptEngineData->EntityClass = ScriptClass("Volcano", "Entity", true);
	}

	// �ű������ȡScene
	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_ScriptEngineData->SceneContext = scene;
	}

	// �Ѷ�ȡmono�����Ƿ����fullClassName��
	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_ScriptEngineData->EntityClasses.find(fullClassName) != s_ScriptEngineData->EntityClasses.end();
	}

	// ��Ӧ��ͬʵ��idʵ����mono��entity���ಢע���ֶ�����
	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();	// �õ����ʵ��Ľű����
		// LoadAssemblyClasses�����ж�ȡmono��
		if (ScriptEngine::EntityClassExists(sc.ClassName))			// �ű�����������Ƿ��Ѷ�ȡmono��
		{
			UUID entityID = entity.GetUUID();
			// ʵ���������
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_ScriptEngineData->EntityClasses[sc.ClassName], entity);
			s_ScriptEngineData->EntityInstances[entityID] = instance;// map�洢(�����)
			
			// ��ȡID��Ӧ�ֶ�
			// LoadAssemblyClasses�����ж�ȡmono����ֶβ������map
			if (s_ScriptEngineData->EntityScriptFields.find(entityID) != s_ScriptEngineData->EntityScriptFields.end())
			{
				const ScriptFieldMap& fieldMap = s_ScriptEngineData->EntityScriptFields.at(entityID);
				for (const auto& [name, fieldInstance] : fieldMap)
					instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
			}
			
			instance->InvokeOnCreate();	// ����C#��OnCreate����
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_ScriptEngineData->EntityInstances.find(entityUUID) != s_ScriptEngineData->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_ScriptEngineData->EntityInstances[entityUUID];
			instance->InvokeOnUpdate((float)ts);
		}
		else
		{
			VOL_CORE_ERROR("Could not find ScriptInstance for entity {}", (uint64_t)entityUUID);
		}
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_ScriptEngineData->SceneContext;
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		auto it = s_ScriptEngineData->EntityInstances.find(entityID);
		if (it == s_ScriptEngineData->EntityInstances.end())
			return nullptr;

		return it->second;
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		if (s_ScriptEngineData->EntityClasses.find(name) == s_ScriptEngineData->EntityClasses.end())
			return nullptr;

		return s_ScriptEngineData->EntityClasses.at(name);
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_ScriptEngineData->SceneContext = nullptr;
		s_ScriptEngineData->EntityInstances.clear();
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return s_ScriptEngineData->EntityClasses;
	}

	// ��ȡʵ���Ӧmono���ֶ�map
	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		VOL_CORE_ASSERT(entity, "ScriptEngine::GetScriptFieldMap");

		UUID entityID = entity.GetUUID();
		return s_ScriptEngineData->EntityScriptFields[entityID];
	}

	// ��ȡC#���򼯵��ಢ���浽map
	void ScriptEngine::LoadAssemblyClasses()
	{
		// ��ʼ��map
		s_ScriptEngineData->EntityClasses.clear();

		// ��ȡ���򼯱�
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_ScriptEngineData->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		// ��ȡCore�����е�Entity��
		MonoClass* entityClass = mono_class_from_name(s_ScriptEngineData->CoreAssemblyImage, "Volcano", "Entity");

		// �������򼯱�
		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_ScriptEngineData->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(s_ScriptEngineData->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;

			// ���������ռ䡢������MonoImage�õ�����C#��Mono��=>�������Ϊ������Class
			MonoClass* monoClass = mono_class_from_name(s_ScriptEngineData->AppAssemblyImage, nameSpace, className);

			// �����õ����ȫ����ΪVolcano.Entity��������
			if (monoClass == entityClass)
				continue;
			
			// �ҵ�dll�����м̳�Entity���࣬�������ǽű��࣬�õ���Ӧ�ķ�װ��Mono�࣬�Ž�Map
			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);

			if (!isEntity)
				continue;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			// ��mono��Ž�data��map��
			s_ScriptEngineData->EntityClasses[fullName] = scriptClass;
			VOL_TRACE(fullName);
			
			// This routine is an iterator routine for retrieving the fields in a class.
			// You must pass a gpointer that points to zero and is treated as an opaque handle
			// to iterate over all of the elements. When no more values are available, the return value is NULL.

			//��������һ�����������̣����ڼ������е��ֶΡ� 
			//�����봫��һ��ָ���㲢����Ϊ��͸�������gpointer 
			//��������Ԫ�ء���û�и������ֵʱ������ֵΪNULL

			// ��ȡmono���ж����ֶ�
			int fieldCount = mono_class_num_fields(monoClass);
			VOL_CORE_WARN("{} has {} fields:", className, fieldCount);
			void* iterator = nullptr;

			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if (flags & MONO_FIELD_ATTR_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
					VOL_CORE_WARN("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

					// ���ֶ�ע��mono����ֶ�map
					scriptClass->m_Fields[fieldName] = { fieldType, fieldName, field };
				}
			}
		}

		auto& entityClasses = s_ScriptEngineData->EntityClasses;

		//mono_field_get_value()
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_ScriptEngineData->CoreAssemblyImage;
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		VOL_CORE_ASSERT(s_ScriptEngineData->EntityInstances.find(uuid) != s_ScriptEngineData->EntityInstances.end());
		return s_ScriptEngineData->EntityInstances.at(uuid)->GetManagedObject();
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_ScriptEngineData->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	//===================ScriptClass=================

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(isCore? s_ScriptEngineData->CoreAssemblyImage : s_ScriptEngineData->AppAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, params, &exception);
	}

	//=======================ScriptInstance==========================

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		// ��ȡSandbox Player�๹�ɵ�MonoObject�����൱��new Sandbox.Player()
		m_Instance = scriptClass->Instantiate();

		m_Constructor = s_ScriptEngineData->EntityClass.GetMethod(".ctor", 1);	// ��ȡC#Entity��Ĺ��캯��
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);				// ��ȡ���洢Sandbox.Player��ĺ���
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		// Call Entity constructor
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);		// ��һ�������������Entity����(Player)���ɵ�mono����
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		if (m_OnUpdateMethod)
		{
			void* param = &ts;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}

	// ��ȡmono����name�ֶε�ֵע��buffer
	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;
	}

	// ��valueע��mono����name�ֶ�
	bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;
	}
}