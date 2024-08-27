#include "volpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/attrdefs.h"

namespace Volcano {

	// 由于C#类类型名称有点不太一眼看出什么意思，所以要自定义类类型，并用map来进行C#类型名称标识转换。
	// 比如：C#的float空间名 + 类名是->System.Single，需转换我们自定义的类名称“Float”。
	// 自定义的类名称需接近C++类名，容易标识。

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

		//  load a file into an array of bytes
		char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				// Failed to open the file
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = end - stream.tellg();

			if (size == 0)
			{
				// File is empty
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = size;
			return buffer;
		}

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				return nullptr;
			}

			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			// free the file data
			delete[] fileData;

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

		const char* ScriptFieldTypeToString(ScriptFieldType type)
		{
			switch (type)
			{
				case ScriptFieldType::Float:   return "Float";
				case ScriptFieldType::Double:  return "Double";
				case ScriptFieldType::Bool:    return "Bool";
				case ScriptFieldType::Char:    return "Char";
				case ScriptFieldType::Byte:    return "Byte";
				case ScriptFieldType::Short:   return "Short";
				case ScriptFieldType::Int:     return "Int";
				case ScriptFieldType::Long:    return "Long";
				case ScriptFieldType::UByte:   return "UByte";
				case ScriptFieldType::UShort:  return "UShort";
				case ScriptFieldType::UInt:    return "UInt";
				case ScriptFieldType::ULong:   return "ULong";
				case ScriptFieldType::Vector2: return "Vector2";
				case ScriptFieldType::Vector3: return "Vector3";
				case ScriptFieldType::Vector4: return "Vector4";
				case ScriptFieldType::Entity:  return "Entity";
			}
			return "<Invalid>";
		}


	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		// Runtime
		Scene* SceneContext = nullptr;
	};
	static ScriptEngineData* s_ScriptEngineData = nullptr;

	void ScriptEngine::Init()
	{
		s_ScriptEngineData = new ScriptEngineData();

		// 1 初始化mono
		InitMono();

		// 2.加载c#程序集
		LoadAssembly("Resources/Scripts/Volcano-ScriptCore.dll");
		LoadAppAssembly("SandBoxProject/Assets/Scripts/Binaries/Sandbox.dll");
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		// 添加内部调用
		ScriptGlue::RegisterFunctions();

		// 创建加载Entity父类-为了在调用OnCreate函数之前把UUID传给C#Entity的构造函数
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
		// 没有MONO_PATH环境变量，默认相对于当前工作目录(VolcanoNut)的路径 
		mono_set_assemblies_path("../Volcano/vendor/mono/lib");

		// 声明根域
		MonoDomain* rootDomain = mono_jit_init("VolcanoJITRuntime");
		if (rootDomain == nullptr)
		{
			// Maybe log some error here
			return;
		}
		// 存储root domain指针
		s_ScriptEngineData->RootDomain = rootDomain;

	}

	void ScriptEngine::ShutdownMono()
	{
		// mono_domain_unload(s_ScriptEngineData->AppDomain);
		s_ScriptEngineData->AppDomain = nullptr;

		// mono_jit_cleanup(s_ScriptEngineData->RootDomain);
		s_ScriptEngineData->RootDomain = nullptr;
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_ScriptEngineData->AppDomain = mono_domain_create_appdomain((char*)"VolcanoScriptRuntime", nullptr);
		mono_domain_set(s_ScriptEngineData->AppDomain, true);

		// 加载c#项目导出的dll
		s_ScriptEngineData->CoreAssembly = Utils::LoadMonoAssembly(filepath);
		// 得到MonoImage对象
		s_ScriptEngineData->CoreAssemblyImage = mono_assembly_get_image(s_ScriptEngineData->CoreAssembly);
		Utils::PrintAssemblyTypes(s_ScriptEngineData->CoreAssembly);// 打印dll的基本信息
	}

	void ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		// Move this maybe
		s_ScriptEngineData->AppAssembly = Utils::LoadMonoAssembly(filepath);
		auto assemb = s_ScriptEngineData->AppAssembly;
		s_ScriptEngineData->AppAssemblyImage = mono_assembly_get_image(s_ScriptEngineData->AppAssembly);
		auto assembi = s_ScriptEngineData->AppAssemblyImage;
		Utils::PrintAssemblyTypes(s_ScriptEngineData->AppAssembly);
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_ScriptEngineData->SceneContext = scene;
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_ScriptEngineData->EntityClasses.find(fullClassName) != s_ScriptEngineData->EntityClasses.end();
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();	// 得到这个实体的组件
		if (ScriptEngine::EntityClassExists(sc.ClassName))			// 组件的脚本名称是否正确
		{
			// 实例化类对象，并存储OnCreate、OnUpdate函数，调用父类Entity的构造函数，传入实体的UUID
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_ScriptEngineData->EntityClasses[sc.ClassName], entity);
			s_ScriptEngineData->EntityInstances[entity.GetUUID()] = instance;// 运行脚本map存储这些ScriptInstance(类对象)
			instance->InvokeOnCreate();										 // 调用C#的OnCreate函数
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		VOL_CORE_ASSERT(s_ScriptEngineData->EntityInstances.find(entityUUID) != s_ScriptEngineData->EntityInstances.end());

		Ref<ScriptInstance> instance = s_ScriptEngineData->EntityInstances[entityUUID];
		instance->InvokeOnUpdate((float)ts);
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

	void ScriptEngine::OnRuntimeStop()
	{
		s_ScriptEngineData->SceneContext = nullptr;
		s_ScriptEngineData->EntityInstances.clear();
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return s_ScriptEngineData->EntityClasses;
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		// 初始化map
		s_ScriptEngineData->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_ScriptEngineData->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(s_ScriptEngineData->CoreAssemblyImage, "Volcano", "Entity");

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

			// 根据命名空间、类名、MonoImage得到加载C#的Mono类=>可以理解为创建类Class
			MonoClass* monoClass = mono_class_from_name(s_ScriptEngineData->AppAssemblyImage, nameSpace, className);

			// 如果获得的类的全类名为Volcano.Entity，则跳过
			if (monoClass == entityClass)
				continue;
			
			// 找到dll里所有继承Entity的类，表明这是脚本类，得到对应的封装的Mono类，放进Map
			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);

			if (!isEntity)
				continue;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			s_ScriptEngineData->EntityClasses[fullName] = scriptClass;
			VOL_TRACE(fullName);
			
			// This routine is an iterator routine for retrieving the fields in a class.
			// You must pass a gpointer that points to zero and is treated as an opaque handle
			// to iterate over all of the elements. When no more values are available, the return value is NULL.

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
		return mono_runtime_invoke(method, instance, params, nullptr);
	}

	//=======================ScriptInstance==========================

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		// 获取Sandbox Player类构成的MonoObject对象，相当于new Sandbox.Player()
		m_Instance = scriptClass->Instantiate();

		m_Constructor = s_ScriptEngineData->EntityClass.GetMethod(".ctor", 1);	// 获取C#Entity类的构造函数
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);				// 获取并存储Sandbox.Player类的函数
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		// Call Entity constructor
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);		// 第一个参数传入的是Entity子类(Player)构成的mono对象
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