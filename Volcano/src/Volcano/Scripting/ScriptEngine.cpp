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

	// C#的float空间名 + 类名是->System.Single，需转换我们自定义的类名称“Float”。
	// 自定义C#类类型名称
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

			// 设置debug
			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");
				if (std::filesystem::exists(pdbPath))
				{
					ScopedBuffer pdbFileData = FileSystem::ReadFileBinary(pdbPath);
					mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), (uint32_t)pdbFileData.Size());
					VOL_CORE_INFO("Loaded PDB：{0} ", pdbPath.generic_string());
				}
			}
			
			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			return assembly;
		}

		// 打印程序集读取了哪些类
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

		// C#类类型转C++类类型标记
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
		// Mono根域
		MonoDomain* RootDomain = nullptr;
		// Mono App域
		MonoDomain* AppDomain = nullptr;
		// Mono Mscorlib域
		//MonoDomain* MscorlibDomain = nullptr;

		// Core程序集
		MonoAssembly* CoreAssembly = nullptr;
		// Core程序集数据
		MonoImage* CoreAssemblyImage = nullptr;

		// App程序集
		MonoAssembly* AppAssembly = nullptr;
		// App程序集数据
		MonoImage* AppAssemblyImage = nullptr;

		// Mscorlib程序集
		//MonoAssembly* MscorlibAssembly = nullptr;
		// Mscorlib程序集数据
		//MonoImage* MscorlibAssemblyImage = nullptr;

		// Core程序集dll文件路径
		std::filesystem::path CoreAssemblyFilepath;
		// App程序集dll文件路径
		std::filesystem::path AppAssemblyFilepath;

		// 一个实体ID对应复数个字段
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		// C#Entity类实例
		Ref<ScriptClass> EntityClass;

		// C#中的Entity子类，如Player
		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		// ID对应的C#中的Entity子类实例，如Player
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		// C#Component类实例
		Ref<ScriptClass> ComponentClass;
		// C#中的Component子类，如TransformComponent
		std::unordered_map<std::string, Ref<ScriptClass>> ComponentClasses;
		// ID对应的C#中的Entity子类实例，如Player
		std::unordered_map<UUID, Ref<ScriptInstance>> ComponentInstances;

		std::vector<InvokeDelayedData> EntityInvokeDelayedList;


		// 文件监管器
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

	// 文件监视器事件，如果状态为修改，则把 重加载C#程序集方法 加入Application主线程队列
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

		// 1 初始化mono
		InitMono();

		// 添加内部调用
		ScriptGlue::RegisterFunctions();

		// 2.加载c#程序集Core
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
		// 没有MONO_PATH环境变量，默认相对于当前工作目录(VolcanoNut)的路径 
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
		
		// 声明根域
		MonoDomain* rootDomain = mono_jit_init("VolcanoJITRuntime");
		if (rootDomain == nullptr)
		{
			// Maybe log some error here
			return;
		}
		// 存储root domain指针
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

	// 读取C# dll文件
	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_ScriptEngineData->AppDomain = mono_domain_create_appdomain((char*)"VolcanoScriptRuntime", nullptr);
		mono_domain_set(s_ScriptEngineData->AppDomain, true);

		s_ScriptEngineData->CoreAssemblyFilepath = filepath;
		// 加载c#项目导出的dll
		s_ScriptEngineData->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_ScriptEngineData->EnableDebugging);
		if (s_ScriptEngineData->CoreAssembly == nullptr)
			return false;
		// 得到MonoImage对象
		s_ScriptEngineData->CoreAssemblyImage = mono_assembly_get_image(s_ScriptEngineData->CoreAssembly);
		//Utils::PrintAssemblyTypes(s_ScriptEngineData->CoreAssembly);// 打印dll的基本信息

		/*
		s_ScriptEngineData->MscorlibAssembly = Utils::LoadMonoAssembly("C:/C++Project/engine/Volcano/Volcano/vendor/mono/lib/mono/4.5/mscorlib.dll", s_ScriptEngineData->EnableDebugging);
		if (s_ScriptEngineData->MscorlibAssembly == nullptr)
			return false;
		// 得到MonoImage对象
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

		// 设置文件监视器
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
		    // 创建加载Entity父类-为了在调用Start函数之前把UUID传给C#Entity的构造函数
			s_ScriptEngineData->EntityClass = std::make_shared<ScriptClass>("Volcano", "Entity", true);
			{
				// 获取Entity类的字段
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

						// 把字段注入mono类的字段map
						s_ScriptEngineData->EntityClass->m_Fields[fieldName] = { fieldType, fieldName, field };
					}
				}

				// 获取Entity类的方法
				int methodCount = mono_class_num_methods(s_ScriptEngineData->EntityClass->GetClass());
				VOL_CORE_WARN(" {} methods:", methodCount);
				void* methodIterator = nullptr;

				while (MonoMethod* method = mono_class_get_methods(s_ScriptEngineData->EntityClass->GetClass(), &methodIterator))
				{
					const char* methodName = mono_method_get_name(method);
					VOL_CORE_WARN("  {}", methodName);
					// 把字段注入mono类的字段map
					s_ScriptEngineData->EntityClass->m_Methods[methodName] = { methodName, method };
				}
			}


			// 创建加载Component类实例
			s_ScriptEngineData->ComponentClass = std::make_shared<ScriptClass>("Volcano", "Component", true);
			{
				// 获取Component类的字段
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

						// 把字段注入mono类的字段map
						s_ScriptEngineData->ComponentClass->m_Fields[fieldName] = { fieldType, fieldName, field };
					}
				}

				// 获取Component类的方法
				int methodCount = mono_class_num_methods(s_ScriptEngineData->ComponentClass->GetClass());
				VOL_CORE_WARN(" {} methods:", methodCount);
				void* methodIterator = nullptr;

				while (MonoMethod* method = mono_class_get_methods(s_ScriptEngineData->ComponentClass->GetClass(), &methodIterator))
				{
					const char* methodName = mono_method_get_name(method);
					VOL_CORE_WARN("  {}", methodName);
					// 把字段注入mono类的字段map
					s_ScriptEngineData->ComponentClass->m_Methods[methodName] = { methodName, method };
				}
			}

		}
	}

	// 脚本引擎获取Scene
	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_ScriptEngineData->SceneContext = scene;
	}

	// 已读取mono类中是否存在fullClassName类
	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_ScriptEngineData->EntityClasses.find(fullClassName) != s_ScriptEngineData->EntityClasses.end();
	}

	// 对应不同实体id实例化mono的entity子类并注入字段数据
	void ScriptEngine::CreateEntity(Entity entity, bool enable)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();	// 得到这个实体的脚本组件
		// LoadAssemblyClasses方法中读取mono类
		if (ScriptEngine::EntityClassExists(sc.ClassName))			// 脚本组件的类名是否已读取mono类
		{
			UUID entityID = entity.GetUUID();
			// 实例化类对象
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_ScriptEngineData->EntityClasses[sc.ClassName], entity, enable);
			s_ScriptEngineData->EntityInstances[entityID] = instance;// map存储(类对象)
			
			// 将ScriptFieldInstance中已有的字段数据保存到C#中，SceneHierarchyPanel中会对ScriptFieldInstance进行修改
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
			instance->InvokeAwake();	// 调用C#的Awake函数
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
				instance->InvokeOnEnable();	// 调用C#的OnEnable函数
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
				instance->InvokeStart();	// 调用C#的Start函数
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
			instance->InvokeOnDisable();	// 调用C#的OnDisable函数
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

	// 获取实体对应mono类字段map
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

	// 读取C#程序集的类并保存到map
	void ScriptEngine::LoadAssemblyClasses()
	{
		// 初始化map
		s_ScriptEngineData->EntityClasses.clear();
		s_ScriptEngineData->ComponentClasses.clear();

		// 获取程序集表
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_ScriptEngineData->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		// 获取Core程序集中的Entity类
		MonoClass* entityClass = mono_class_from_name(s_ScriptEngineData->CoreAssemblyImage, "Volcano", "Entity");
		// 获取Core程序集中的Component类
		MonoClass* componentClass = mono_class_from_name(s_ScriptEngineData->CoreAssemblyImage, "Volcano", "Component");

		// 遍历程序集表
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
			if (monoClass == entityClass || monoClass == componentClass)
			{
				continue;
			}
			
			// 找到dll里所有继承Entity的类，表明这是脚本类，得到对应的封装的Mono类，放进Map
			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			bool isComponent = mono_class_is_subclass_of(monoClass, componentClass, false);

			if (!(isEntity || isComponent))
				continue;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);

			// 把mono类放进data的map里
			if(isEntity)
			    s_ScriptEngineData->EntityClasses[fullName] = scriptClass;
			else if(isComponent)
				s_ScriptEngineData->ComponentClasses[fullName] = scriptClass;


			// This routine is an iterator routine for retrieving the fields in a class.
			// You must pass a gpointer that points to zero and is treated as an opaque handle
			// to iterate over all of the elements. When no more values are available, the return value is NULL.

			//此例程是一个迭代器例程，用于检索类中的字段。 
			//您必须传递一个指向零并被视为不透明句柄的gpointer 
			//迭代所有元素。当没有更多可用值时，返回值为NULL

			VOL_CORE_WARN("{}.{}:", nameSpace, className);
			// 获取mono类有多少字段
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

					// 把字段注入mono类的字段map
					scriptClass->m_Fields[fieldName] = { fieldType, fieldName, field };
				}
			}
			// 获取mono类有多少方法
			int methodCount = mono_class_num_methods(monoClass);
			VOL_CORE_WARN(" {} methods:", methodCount);
			void* methodIterator = nullptr;

			while (MonoMethod* method = mono_class_get_methods(monoClass, &methodIterator))
			{
				const char* methodName = mono_method_get_name(method);
				VOL_CORE_WARN("  {}", methodName);
				// 把字段注入mono类的字段map
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

	// 方法名，参数数量
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
		// 获取Sandbox Player类构成的MonoObject对象，相当于new Sandbox.Player()
		m_Instance = scriptClass->Instantiate();

		m_Constructor = s_ScriptEngineData->EntityClass->GetMethod(".ctor", 1);	// 获取C#Entity类的构造函数
		m_AwakeMethod = scriptClass->GetMethod("Awake", 0);				// 获取并存储Sandbox.Player类的函数
		m_StartMethod = scriptClass->GetMethod("Start", 0);
		m_UpdateMethod = scriptClass->GetMethod("Update", 1);
		m_OnEnableMethod = scriptClass->GetMethod("OnEnable", 0);
		m_OnDisableMethod = scriptClass->GetMethod("OnDisable", 0);

		// Call Entity constructor
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);		// 第一个参数传入的是Entity子类(Player)构成的mono对象
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
		m_Constructor = s_ScriptEngineData->EntityClass->GetMethod(".ctor", 1);	// 获取C#Entity类的构造函数
	}

	void ScriptInstance::InvokeAwake() { InvokeMethod(m_AwakeMethod); }
	void ScriptInstance::InvokeStart() { InvokeMethod(m_StartMethod); }
	void ScriptInstance::InvokeUpdate(float ts) { void* param = &ts; InvokeMethod(m_UpdateMethod, &param); }
	void ScriptInstance::InvokeOnEnable() { InvokeMethod(m_OnEnableMethod); }
	void ScriptInstance::InvokeOnDisable() { InvokeMethod(m_OnDisableMethod); }
	void ScriptInstance::InvokeMethod(MonoMethod* method, void** param) { if (method) m_ScriptClass->InvokeMethod(m_Instance, method, param); }

	// 获取mono类中name字段的值注入buffer
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
			// 如果字段为引用类型， buffer返回一个MonoObject*指针
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

	// 将value注入mono类中name字段
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