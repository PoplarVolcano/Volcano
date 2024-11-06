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
#include "Volcano/Project/Project.h"
#include "Volcano/Scene/Prefab.h"

namespace Volcano {

	// C#��float�ռ��� + ������->System.Single����ת�������Զ���������ơ�Float����
	// �Զ���C#����������
	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Single",      ScriptFieldType::Float      },
		{ "System.Double",      ScriptFieldType::Double     },
		{ "System.Boolean",     ScriptFieldType::Bool       },
		{ "System.Char",        ScriptFieldType::Char       },
		{ "System.Int16",       ScriptFieldType::Short      },
		{ "System.Int32",       ScriptFieldType::Int        },
		{ "System.Int64",       ScriptFieldType::Long       },
		{ "System.Byte",        ScriptFieldType::Byte       },
		{ "System.UInt16",      ScriptFieldType::UShort     },
		{ "System.UInt32",      ScriptFieldType::UInt       },
		{ "System.UInt64",      ScriptFieldType::ULong      },
		{ "System.String",      ScriptFieldType::String     },
							  	 					 	    
		{ "Volcano.Vector2",    ScriptFieldType::Vector2    },
		{ "Volcano.Vector3",    ScriptFieldType::Vector3    },
		{ "Volcano.Vector4",    ScriptFieldType::Vector4    },
		{ "Volcano.Quaternion", ScriptFieldType::Quaternion },
		{ "Volcano.Matrix4x4",  ScriptFieldType::Matrix4x4  },
													 
		{ "Volcano.Entity",     ScriptFieldType::Entity     },
	};

	namespace Utils {

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			ScopedBuffer fileData = FileSystem::ReadFileBinary(assemblyPath);

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), (uint32_t)fileData.Size(), 1, &status, 0);

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
					mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), (uint32_t)pdbFileData.Size());
					VOL_CORE_INFO("Loaded PDB��{0} ", pdbPath.generic_string());
				}
			}
			
			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			return assembly;
		}

		// ��ӡ���򼯶�ȡ����Щ��
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
		// Mono Mscorlib��
		//MonoDomain* MscorlibDomain = nullptr;

		// Core����
		MonoAssembly* CoreAssembly = nullptr;
		// Core��������
		MonoImage* CoreAssemblyImage = nullptr;

		// App����
		MonoAssembly* AppAssembly = nullptr;
		// App��������
		MonoImage* AppAssemblyImage = nullptr;

		// Mscorlib����
		//MonoAssembly* MscorlibAssembly = nullptr;
		// Mscorlib��������
		//MonoImage* MscorlibAssemblyImage = nullptr;

		// Core����dll�ļ�·��
		std::filesystem::path CoreAssemblyFilepath;
		// App����dll�ļ�·��
		std::filesystem::path AppAssemblyFilepath;

		// һ��ʵ��ID��Ӧ�������ֶ�
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		// C#Entity��ʵ��
		Ref<ScriptClass> EntityClass;

		// C#�е�Entity���࣬��Player
		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		// ID��Ӧ��C#�е�Entity����ʵ������Player
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		// C#Component��ʵ��
		Ref<ScriptClass> ComponentClass;
		// C#�е�Component���࣬��TransformComponent
		std::unordered_map<std::string, Ref<ScriptClass>> ComponentClasses;
		// ID��Ӧ��C#�е�Entity����ʵ������Player
		std::unordered_map<UUID, Ref<ScriptInstance>> ComponentInstances;

		std::vector<InvokeDelayedData> EntityInvokeDelayedList;


		// �ļ������
		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
		bool AssemblyReloadPending = false;

#ifdef VOL_DEBUG
		bool EnableDebugging = true;
#else
		bool EnableDebugging = false;
#endif

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

		// 2.����c#����Core
		bool status = LoadAssembly("Resources/Scripts/Volcano-ScriptCore.dll");
		if (!status)
		{
			VOL_CORE_ERROR("[ScriptEngine] Could not load Volcano-ScriptCore assembly.");
			return;
		}
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

		/*
		s_ScriptEngineData->MscorlibAssembly = Utils::LoadMonoAssembly("C:/C++Project/engine/Volcano/Volcano/vendor/mono/lib/mono/4.5/mscorlib.dll", s_ScriptEngineData->EnableDebugging);
		if (s_ScriptEngineData->MscorlibAssembly == nullptr)
			return false;
		// �õ�MonoImage����
		s_ScriptEngineData->MscorlibAssemblyImage = mono_assembly_get_image(s_ScriptEngineData->MscorlibAssembly);
		*/
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

	void ScriptEngine::SetAppAssembly(const std::filesystem::path& filepath)
	{
		s_ScriptEngineData->AppAssemblyFilepath = filepath;
	}

	void ScriptEngine::ReloadAssembly()
	{
		//mono_domain_set(mono_get_root_domain(), false);
		//mono_domain_unload(s_ScriptEngineData->AppDomain);
		//LoadAssembly(s_ScriptEngineData->CoreAssemblyFilepath);

		s_ScriptEngineData->EntityClass = nullptr;
		s_ScriptEngineData->EntityClasses.clear();
		s_ScriptEngineData->EntityInstances.clear();
		s_ScriptEngineData->EntityScriptFields.clear();
		s_ScriptEngineData->ComponentClass = nullptr;
		s_ScriptEngineData->ComponentClasses.clear();
		s_ScriptEngineData->ComponentInstances.clear();

		auto scriptModulePath = Project::GetAssetDirectory() / Project::GetActive()->GetConfig().ScriptModulePath;
		if (!Project::GetActive()->GetConfig().ScriptModulePath.empty() && std::filesystem::exists(scriptModulePath))
		{
			bool status = LoadAppAssembly(scriptModulePath);
			if (!status)
			{
				VOL_CORE_ERROR("[ScriptEngine] Could not load app assembly.");
				return;
			}
			LoadAssemblyClasses();
			ScriptGlue::RegisterComponents();

		    // Retrieve and instantiate class
		    // ��������Entity����-Ϊ���ڵ���Start����֮ǰ��UUID����C#Entity�Ĺ��캯��
			s_ScriptEngineData->EntityClass = std::make_shared<ScriptClass>("Volcano", "Entity", true);
			{
				// ��ȡEntity����ֶ�
				int fieldCount = mono_class_num_fields(s_ScriptEngineData->EntityClass->GetClass());
				VOL_CORE_WARN("{}.{} has {} fields:", "Volcano", "Entity", fieldCount);
				void* iterator = nullptr;

				while (MonoClassField* field = mono_class_get_fields(s_ScriptEngineData->EntityClass->GetClass(), &iterator))
				{
					const char* fieldName = mono_field_get_name(field);
					uint32_t flags = mono_field_get_flags(field);
					if (flags & MONO_FIELD_ATTR_PUBLIC)
					{
						MonoType* type = mono_field_get_type(field);
						ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
						VOL_CORE_WARN("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

						// ���ֶ�ע��mono����ֶ�map
						s_ScriptEngineData->EntityClass->m_Fields[fieldName] = { fieldType, fieldName, field };
					}
				}

				// ��ȡEntity��ķ���
				int methodCount = mono_class_num_methods(s_ScriptEngineData->EntityClass->GetClass());
				VOL_CORE_WARN(" {} methods:", methodCount);
				void* methodIterator = nullptr;

				while (MonoMethod* method = mono_class_get_methods(s_ScriptEngineData->EntityClass->GetClass(), &methodIterator))
				{
					const char* methodName = mono_method_get_name(method);
					VOL_CORE_WARN("  {}", methodName);
					// ���ֶ�ע��mono����ֶ�map
					s_ScriptEngineData->EntityClass->m_Methods[methodName] = { methodName, method };
				}
			}


			// ��������Component��ʵ��
			s_ScriptEngineData->ComponentClass = std::make_shared<ScriptClass>("Volcano", "Component", true);
			{
				// ��ȡComponent����ֶ�
				int fieldCount = mono_class_num_fields(s_ScriptEngineData->ComponentClass->GetClass());
				VOL_CORE_WARN("{}.{} has {} fields:", "Volcano", "Component", fieldCount);
				void* iterator = nullptr;

				while (MonoClassField* field = mono_class_get_fields(s_ScriptEngineData->ComponentClass->GetClass(), &iterator))
				{
					const char* fieldName = mono_field_get_name(field);
					uint32_t flags = mono_field_get_flags(field);
					if (flags & MONO_FIELD_ATTR_PUBLIC)
					{
						MonoType* type = mono_field_get_type(field);
						ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
						VOL_CORE_WARN("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

						// ���ֶ�ע��mono����ֶ�map
						s_ScriptEngineData->ComponentClass->m_Fields[fieldName] = { fieldType, fieldName, field };
					}
				}

				// ��ȡComponent��ķ���
				int methodCount = mono_class_num_methods(s_ScriptEngineData->ComponentClass->GetClass());
				VOL_CORE_WARN(" {} methods:", methodCount);
				void* methodIterator = nullptr;

				while (MonoMethod* method = mono_class_get_methods(s_ScriptEngineData->ComponentClass->GetClass(), &methodIterator))
				{
					const char* methodName = mono_method_get_name(method);
					VOL_CORE_WARN("  {}", methodName);
					// ���ֶ�ע��mono����ֶ�map
					s_ScriptEngineData->ComponentClass->m_Methods[methodName] = { methodName, method };
				}
			}

		}
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
	void ScriptEngine::CreateEntity(Entity entity, bool enable)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();	// �õ����ʵ��Ľű����
		// LoadAssemblyClasses�����ж�ȡmono��
		if (ScriptEngine::EntityClassExists(sc.ClassName))			// �ű�����������Ƿ��Ѷ�ȡmono��
		{
			UUID entityID = entity.GetUUID();
			// ʵ���������
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_ScriptEngineData->EntityClasses[sc.ClassName], entity, enable);
			s_ScriptEngineData->EntityInstances[entityID] = instance;// map�洢(�����)
			
			// ��ScriptFieldInstance�����е��ֶ����ݱ��浽C#�У�SceneHierarchyPanel�л��ScriptFieldInstance�����޸�
			if (s_ScriptEngineData->EntityScriptFields.find(entityID) != s_ScriptEngineData->EntityScriptFields.end())
			{
				auto& fieldIDMap = s_ScriptEngineData->EntityScriptFields;
				VOL_CORE_ASSERT(fieldIDMap.find(entityID) != fieldIDMap.end());
				const ScriptFieldMap& fieldMap = fieldIDMap.at(entityID);
				for (const auto& [name, fieldInstance] : fieldMap)
					instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
			}
		}
	}

	void ScriptEngine::AwakeEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_ScriptEngineData->EntityInstances.find(entityUUID) != s_ScriptEngineData->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_ScriptEngineData->EntityInstances[entityUUID];
			instance->InvokeAwake();	// ����C#��Awake����
		}
		else
		{
			VOL_CORE_ERROR("Could not find ScriptInstance for entity {}", (uint64_t)entityUUID);
		}
	}

	void ScriptEngine::OnEnableEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_ScriptEngineData->EntityInstances.find(entityUUID) != s_ScriptEngineData->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_ScriptEngineData->EntityInstances[entityUUID];
			if (instance->GetEnable())
			{
				instance->InvokeOnEnable();	// ����C#��OnEnable����
			}
		}
		else
		{
			VOL_CORE_ERROR("Could not find ScriptInstance for entity {}", (uint64_t)entityUUID);
		}
	}

	void ScriptEngine::StartEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_ScriptEngineData->EntityInstances.find(entityUUID) != s_ScriptEngineData->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_ScriptEngineData->EntityInstances[entityUUID];
			if (instance->GetEnable())
			{
				instance->InvokeStart();	// ����C#��Start����
			}
		}
		else
		{
			VOL_CORE_ERROR("Could not find ScriptInstance for entity {}", (uint64_t)entityUUID);
		}
	}

	void ScriptEngine::UpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_ScriptEngineData->EntityInstances.find(entityUUID) != s_ScriptEngineData->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_ScriptEngineData->EntityInstances[entityUUID];
			if (instance->GetEnable())
			{
			    instance->InvokeUpdate((float)ts);
			}
		}
		else
		{
			VOL_CORE_ERROR("Could not find ScriptInstance for entity {}", (uint64_t)entityUUID);
		}
	}

	void ScriptEngine::OnDisableEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		if (s_ScriptEngineData->EntityInstances.find(entityUUID) != s_ScriptEngineData->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_ScriptEngineData->EntityInstances[entityUUID];
			instance->InvokeOnDisable();	// ����C#��OnDisable����
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

	Ref<ScriptClass> ScriptEngine::GetEntityClass()
	{
		return s_ScriptEngineData->EntityClass;
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

	std::vector<InvokeDelayedData>& ScriptEngine::GetEntityInvokeDelayedList()
	{
		return s_ScriptEngineData->EntityInvokeDelayedList;
	}

	// ��ȡC#���򼯵��ಢ���浽map
	void ScriptEngine::LoadAssemblyClasses()
	{
		// ��ʼ��map
		s_ScriptEngineData->EntityClasses.clear();
		s_ScriptEngineData->ComponentClasses.clear();

		// ��ȡ���򼯱�
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_ScriptEngineData->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		// ��ȡCore�����е�Entity��
		MonoClass* entityClass = mono_class_from_name(s_ScriptEngineData->CoreAssemblyImage, "Volcano", "Entity");
		// ��ȡCore�����е�Component��
		MonoClass* componentClass = mono_class_from_name(s_ScriptEngineData->CoreAssemblyImage, "Volcano", "Component");

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
			if (monoClass == entityClass || monoClass == componentClass)
			{
				continue;
			}
			
			// �ҵ�dll�����м̳�Entity���࣬�������ǽű��࣬�õ���Ӧ�ķ�װ��Mono�࣬�Ž�Map
			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			bool isComponent = mono_class_is_subclass_of(monoClass, componentClass, false);

			if (!(isEntity || isComponent))
				continue;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);

			// ��mono��Ž�data��map��
			if(isEntity)
			    s_ScriptEngineData->EntityClasses[fullName] = scriptClass;
			else if(isComponent)
				s_ScriptEngineData->ComponentClasses[fullName] = scriptClass;


			// This routine is an iterator routine for retrieving the fields in a class.
			// You must pass a gpointer that points to zero and is treated as an opaque handle
			// to iterate over all of the elements. When no more values are available, the return value is NULL.

			//��������һ�����������̣����ڼ������е��ֶΡ� 
			//�����봫��һ��ָ���㲢����Ϊ��͸�������gpointer 
			//��������Ԫ�ء���û�и������ֵʱ������ֵΪNULL

			VOL_CORE_WARN("{}.{}:", nameSpace, className);
			// ��ȡmono���ж����ֶ�
			int fieldCount = mono_class_num_fields(monoClass);
			VOL_CORE_WARN(" {} fields:", fieldCount);
			void* fieldIterator = nullptr;

			while (MonoClassField* field = mono_class_get_fields(monoClass, &fieldIterator))
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
			// ��ȡmono���ж��ٷ���
			int methodCount = mono_class_num_methods(monoClass);
			VOL_CORE_WARN(" {} methods:", methodCount);
			void* methodIterator = nullptr;

			while (MonoMethod* method = mono_class_get_methods(monoClass, &methodIterator))
			{
				const char* methodName = mono_method_get_name(method);
				VOL_CORE_WARN("  {}", methodName);
				// ���ֶ�ע��mono����ֶ�map
				scriptClass->m_Methods[methodName] = { methodName, method };
			}
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_ScriptEngineData->CoreAssemblyImage;
	}

	MonoImage* ScriptEngine::GetAppAssemblyImage()
	{
		return s_ScriptEngineData->AppAssemblyImage;
	}
	/*
	MonoImage* ScriptEngine::GetMscorlibAssemblyImage()
	{
		return s_ScriptEngineData->MscorlibAssemblyImage;
	}
	*/
	MonoDomain* ScriptEngine::GetCoreAssemblyDomain()
	{
		return s_ScriptEngineData->RootDomain;
	}

	MonoDomain* ScriptEngine::GetAppAssemblyDomain()
	{
		return s_ScriptEngineData->AppDomain;
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		VOL_CORE_ASSERT(s_ScriptEngineData->EntityInstances.find(uuid) != s_ScriptEngineData->EntityInstances.end());
		return s_ScriptEngineData->EntityInstances.at(uuid)->GetManagedObject();
	}

	MonoClass* ScriptEngine::GetClass(ScriptFieldType type)
	{
		switch (type)
		{
		case ScriptFieldType::ULong:
			return mono_get_uint64_class();
		}
		return nullptr;
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

	ScriptClass::ScriptClass(MonoClass* monoClass, const std::string& classNamespace, const std::string& className)
		: m_ClassNamespace(classNamespace), m_ClassName(className), m_MonoClass(monoClass)
	{}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	// ����������������
	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	ScriptMethod& ScriptClass::GetMethod(const std::string& name)
	{
		VOL_CORE_ASSERT(m_Methods.find(name) != m_Methods.end());
		return m_Methods.at(name);
	}

	bool ScriptClass::HasMethod(const std::string& name)
	{
		return m_Methods.find(name) != m_Methods.end();
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, params, &exception);
	}

	//=======================ScriptInstance==========================

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity, bool enable)
		: m_ScriptClass(scriptClass), m_Enable(enable)
	{
		// ��ȡSandbox Player�๹�ɵ�MonoObject�����൱��new Sandbox.Player()
		m_Instance = scriptClass->Instantiate();

		m_Constructor = s_ScriptEngineData->EntityClass->GetMethod(".ctor", 1);	// ��ȡC#Entity��Ĺ��캯��
		m_AwakeMethod = scriptClass->GetMethod("Awake", 0);				// ��ȡ���洢Sandbox.Player��ĺ���
		m_StartMethod = scriptClass->GetMethod("Start", 0);
		m_UpdateMethod = scriptClass->GetMethod("Update", 1);
		m_OnEnableMethod = scriptClass->GetMethod("OnEnable", 0);
		m_OnDisableMethod = scriptClass->GetMethod("OnDisable", 0);

		// Call Entity constructor
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);		// ��һ�������������Entity����(Player)���ɵ�mono����
		}
	}

	ScriptInstance::ScriptInstance(MonoObject* instance, bool enable)
		: m_Enable(enable)
	{
		MonoClass* monoClass = mono_object_get_class(instance);

		if (monoClass == s_ScriptEngineData->EntityClass->GetClass())
			m_ScriptClass = s_ScriptEngineData->EntityClass;
		else if (mono_class_is_subclass_of(monoClass, s_ScriptEngineData->EntityClass->GetClass(), false))
		{
			const char* nameSpace = mono_class_get_namespace(monoClass);
			const char* className = mono_class_get_name(monoClass);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;
			if(s_ScriptEngineData->EntityClasses.find(fullName) != s_ScriptEngineData->EntityClasses.end())
			    m_ScriptClass = s_ScriptEngineData->EntityClasses.at(fullName);
		}
		else if(monoClass == s_ScriptEngineData->ComponentClass->GetClass())
			m_ScriptClass = s_ScriptEngineData->ComponentClass;
		else if (mono_class_is_subclass_of(monoClass, s_ScriptEngineData->ComponentClass->GetClass(), false))
		{
			const char* nameSpace = mono_class_get_namespace(monoClass);
			const char* className = mono_class_get_name(monoClass);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;
			if (s_ScriptEngineData->ComponentClasses.find(fullName) != s_ScriptEngineData->ComponentClasses.end())
				m_ScriptClass = s_ScriptEngineData->ComponentClasses.at(fullName);
		}
		VOL_CORE_ASSERT(m_ScriptClass != nullptr);
		m_Instance = instance;
		m_Constructor = s_ScriptEngineData->EntityClass->GetMethod(".ctor", 1);	// ��ȡC#Entity��Ĺ��캯��
	}

	void ScriptInstance::InvokeAwake() { InvokeMethod(m_AwakeMethod); }
	void ScriptInstance::InvokeStart() { InvokeMethod(m_StartMethod); }
	void ScriptInstance::InvokeUpdate(float ts) { void* param = &ts; InvokeMethod(m_UpdateMethod, &param); }
	void ScriptInstance::InvokeOnEnable() { InvokeMethod(m_OnEnableMethod); }
	void ScriptInstance::InvokeOnDisable() { InvokeMethod(m_OnDisableMethod); }
	void ScriptInstance::InvokeMethod(MonoMethod* method, void** param) { if (method) m_ScriptClass->InvokeMethod(m_Instance, method, param); }

	// ��ȡmono����name�ֶε�ֵע��buffer
	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		switch (field.Type)
		{
		case ScriptFieldType::Entity:
			// ����ֶ�Ϊ�������ͣ� buffer����һ��MonoObject*ָ��
			MonoObject* field_value;
			mono_field_get_value(m_Instance, field.ClassField, &field_value);
			if (field_value == NULL)
			{
				uint64_t id = 0;
				memcpy(buffer, &id, sizeof(id));
			}
			else
			{
				VOL_CORE_ASSERT(ScriptEngine::GetEntityClass()->GetFields().find("ID") != ScriptEngine::GetEntityClass()->GetFields().end());
				mono_field_get_value(field_value, ScriptEngine::GetEntityClass()->GetFields().at("ID").ClassField, buffer);
			}
			break;
		case ScriptFieldType::ULong:
			mono_field_get_value(m_Instance, field.ClassField, buffer);
			break;
		case ScriptFieldType::Float:
			mono_field_get_value(m_Instance, field.ClassField, buffer);
			break;
		}
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

		Ref<Entity> entity;
		switch (field.Type)
		{
		case ScriptFieldType::Entity:
			if(ScriptEngine::GetSceneContext()->GetEntityIDMap().find(*(uint64_t*)value) != ScriptEngine::GetSceneContext()->GetEntityIDMap().end())
			    entity = ScriptEngine::GetSceneContext()->GetEntityIDMap().at(*(uint64_t*)value);
			else if (Prefab::GetScene()->GetEntityIDMap().find(*(uint64_t*)value) != Prefab::GetScene()->GetEntityIDMap().end())
			{
				entity = Prefab::GetScene()->GetEntityIDMap().at(*(uint64_t*)value);
			}
			if (entity != nullptr)
			{
				//ScriptInstance scriptInstanceTemp = ScriptInstance(ScriptEngine::GetEntityClass(), *entity.get());
				//scriptInstanceTemp.SetFieldValue("ID", *(uint64_t*)value);
				mono_field_set_value(m_Instance, field.ClassField, ScriptInstance(ScriptEngine::GetEntityClass(), *entity.get()).GetManagedObject());
			}
			break;
		case ScriptFieldType::ULong:
			mono_field_set_value(m_Instance, field.ClassField, (void*)value);
			break;
		case ScriptFieldType::Float:
			mono_field_set_value(m_Instance, field.ClassField, (void*)value);
			break;
		}
		return true;
	}
}