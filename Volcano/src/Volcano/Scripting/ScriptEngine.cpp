#include "volpch.h"
#include "ScriptEngine.h"

#include "FileWatch.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-debug.h>
#include "mono/metadata/threads.h"

#include "Volcano/Core/Application.h"
#include "Volcano/Project/Project.h"
#include "Volcano/Scripting/ScriptGlue.h"

namespace Volcano
{
    namespace Utils
    {
        /*
        通常在游戏引擎中需要加载两个 DLL：一个由引擎提供，一个包含游戏代码。
        这样引擎开发者可以为用户提供一个安全的 API 来交互。

        将程序集文件以二进制形式读入内存
        mono_image_open_from_data_full：通过内存数据创建一个 镜像（Image）——它代表程序集的元数据结构
        第三个参数告诉 Mono 是否要复制数据。这里传 1，表示 Mono 会将数据复制到其内部缓冲区中
        第四个参数是一个指向 MonoImageOpenStatus 枚举的指针，通过这个值来判断 Mono 是否成功读取数据
        第五个参数是一个布尔值。
        如果设为 true（或 1），表示 Mono 将以“反射模式”加载镜像，这意味着我们可以检查类型，
        但不能运行任何代码。但我们是要运行代码的，所以设为 false（或 0）。

        加载调试符号：
        当 loadPDB 为 true 时，构造对应的 .pdb 文件路径（替换扩展名）。
        如果文件存在，则同样读取其二进制数据，调用 mono_debug_open_image_from_memory 将调试符号信息关联到刚创建的 image 上。
        这样，调试器就能获取源代码行号、局部变量名等信息。
        若找不到 .pdb，则不加载，也不会报错（只是缺少调试信息）。

        mono_assembly_load_from_full：将 MonoImage 转换为 MonoAssembly，并使其成为运行时可执行单元。
        第一个参数是从 Mono 获取的镜像。
        第二个参数只是一个名称，Mono 在打印错误时会用到它。
        第三个参数是 status 变量。如果出错，该函数会写入 status，但此时不应该再有错误了，所以不检查它。
        第四个参数与 mono_image_open_from_data_full 的最后一个参数相同，如果在那里传了 1，这里也应该传 1

        关闭镜像并返回程序集
        */
        MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false);
        void PrintAssemblyTypes(MonoAssembly* assembly);
        uint8_t GetFieldAccessibility(MonoClassField* field);
        uint8_t GetPropertyAccessibility(MonoProperty* property);
        bool CheckMonoError(MonoError& error);
        std::string MonoStringToUTF8(MonoString* monoString);
        ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType);

    }

    static std::vector<std::pair<std::string, std::string>> s_CoreAssamblyClassList =
    {
        { "Volcano","Object"           },
        { "Volcano","GameObject"       },
        { "Volcano","ScriptableObject" },
        { "Volcano","Component"        },
        { "Volcano","Transform"        },
        { "Volcano","Behaviour"        },
        { "Volcano","MonoBehaviour"    }
    };

    struct ScriptEngineData
    {
        MonoDomain* RootDomain = nullptr;
        MonoDomain* AppDomain = nullptr;

        MonoAssembly* CoreAssembly = nullptr;
        MonoImage* CoreAssemblyImage = nullptr;

        MonoAssembly* AppAssembly = nullptr;
        MonoImage* AppAssemblyImage = nullptr;

        std::unordered_map<std::string, Ref<ScriptClass>> CoreScriptClassMap;
        std::unordered_map<std::string, Ref<ScriptClass>> AppScriptClassMap;
        std::unordered_map<UUID, Ref<ScriptInstanceMonoBehaviour>> EntityScriptInstanceMap;
        EntityScriptFieldMap EntityScriptFieldMap; // 一个实体ID对应复数个字段

        ScriptField* IDScriptField;

        std::filesystem::path CoreAssemblyFilepath;
        std::filesystem::path AppAssemblyFilepath;

        // 在ScriptGlue中会把延迟调用的方法被加入EntityInvokeDelayedListBuffer
        // 每次Update结束之后，会把EntityInvokeDelayedListBuffer中的方法转移到EntityInvokeDelayedList
        // 然后对EntityInvokeDelayedList中的方法进行结算，时间戳到期的执行方法
        std::vector<InvokeDelayedData> EntityInvokeDelayedListBuffer;
        std::vector<InvokeDelayedData> EntityInvokeDelayedList;
        
        //在ScriptGlue中会把对Entity进行添加、移动、销毁的操作被加入EntityUpdateList
        // EntityInvokeDelayedList结算后进行EntityUpdateList结算，统一对Entity进行添加、移动、销毁
        std::queue<EntityUpdateBuffer> EntityUpdateList;

        // 文件监管器
        Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
        bool AssemblyReloadPending = false;

        Scene* SceneContext = nullptr;

#ifdef VOL_DEBUG
        bool EnabledDebugging = true;
#else
        bool EnabledDebugging = false;
#endif

    };
    static ScriptEngineData* s_ScriptEngineData = nullptr;

    // 文件监视器事件，如果状态为修改，则把 重加载C#程序集方法 加入Application主线程队列
    static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type)
    {
        if (!s_ScriptEngineData->AssemblyReloadPending && change_type == filewatch::Event::modified)
        {
            s_ScriptEngineData->AssemblyReloadPending = true;

            Application::GetInstance().SubmitToMainThread([]()
                {
                    s_ScriptEngineData->AppAssemblyFileWatcher.reset();
                    ScriptEngine::ReloadAssembly();
                });
        }
    }


    void ScriptEngine::Init()
    {
        s_ScriptEngineData = new ScriptEngineData();

        InitMono();

        ScriptGlue::RegisterFunctions();

        if (!LoadCoreAssembly("Resources/Scripts/VolcanoScriptCore.dll"))
            VOL_CORE_ERROR("[ScriptEngine] Could not load VolcanoScriptCore assembly.");

    }

    void ScriptEngine::Shutdown()
    {
        ShutdownMono();
        delete s_ScriptEngineData;
    }

    /*
    创建一个应用程序域（App Domain），第二个参数允许传入配置文件的路径
    mono_domain_set：将新创建的 App Domain 设置为当前 App Domain

    加载核心程序集
    设置核心程序集镜像

    打印程序集所有的类
    */
    bool ScriptEngine::LoadCoreAssembly(const std::filesystem::path& filepath)
    {
        s_ScriptEngineData->AppDomain = mono_domain_create_appdomain((char*)"VolcanoAppDomain", nullptr);
        mono_domain_set(s_ScriptEngineData->AppDomain, true);
        s_ScriptEngineData->CoreAssemblyFilepath = filepath;
        s_ScriptEngineData->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_ScriptEngineData->EnabledDebugging);
        if (s_ScriptEngineData->CoreAssembly == nullptr)
            return false;
        s_ScriptEngineData->CoreAssemblyImage = mono_assembly_get_image(s_ScriptEngineData->CoreAssembly);
        
        // Utils::PrintAssemblyTypes(s_ScriptEngineData->CoreAssembly);
        VOL_CORE_WARN(filepath.filename().string());

        return true;
    }

    /*
    加载应用程序集
    设置应用程序集镜像

    打印程序集所有的类

    设置文件监视器
    */
    bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
    {
        s_ScriptEngineData->AppAssemblyFilepath = filepath;
        s_ScriptEngineData->AppAssembly = Utils::LoadMonoAssembly(filepath, s_ScriptEngineData->EnabledDebugging);
        if (s_ScriptEngineData->AppAssembly == nullptr)
            return false;
        s_ScriptEngineData->AppAssemblyImage = mono_assembly_get_image(s_ScriptEngineData->AppAssembly);

        //Utils::PrintAssemblyTypes(s_ScriptEngineData->AppAssembly);
        VOL_CORE_WARN(filepath.filename().string());

        s_ScriptEngineData->AppAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(filepath.string(), OnAppAssemblyFileSystemEvent);
        s_ScriptEngineData->AssemblyReloadPending = false;

        return true;
    }

    /*
    获取用户程序集路径，仅当路径非空且文件存在时才执行重载，否则跳过。

    mono_domain_set(mono_get_root_domain(), true)：将当前线程切换到根域（Root Domain），这是 Mono 运行时的初始域。
    第二个参数为 true 时表示将该域设置为当前域；
    通常热重载前需要先离开要卸载的域，因此切换至根域是必要的。
    
    mono_domain_unload(s_ScriptEngineData->AppDomain)：请求卸载当前应用程序域。
    此操作会释放域内的所有程序集、类型实例和静态变量，为重新加载腾出空间。
    注意：域卸载是异步的，可能不会立即完成；且如果域内仍有活跃对象引用，卸载会失败（通常返回 FALSE）
    
    加载应用程序集
    清空旧类型缓存
    加载核心程序集中预定义的类
   
    程序集会把它包含的所有必要信息存储在一系列表（tables）中。
    Mono 允许我们遍历每个表中的所有行，以 MONO_TABLE_TYPEDEF 表为例，每一行代表一个类型，列包含该类型的信息。
    通过调用 mono_image_get_table_info 并传入镜像和所需表的“ID”来从镜像中获取一个表。
    MONO_TABLE_TYPEDEF：类型定义表（type definitions table）。
    通过调用 mono_table_info_get_rows 并传入表信息指针获取该表中的行数（类型定义的数量）

    然后遍历所有行，接下来要获取每一行的所有列值。
    所有列的数据都存储为无符号 32 位整数，所以我们先分配一个名为 cols 的数组，数组大小为所遍历表的最大列数。
    Mono 为每个表提供了对应的常量，所以这里我们将数组大小设为 MONO_TYPEDEF_SIZE。

    通过调用 mono_metadata_decode_row 来解码类型定义表中的当前行，以填充 cols 数组
    第一个参数：我们正在遍历的表。
    第二个参数：要获取列的行索引。
    第三个参数：我们分配好的列数组。
    第四个参数：该数组的大小。
    调用函数后，cols 数组就会被填入一系列值，然后可以用这些值来获取该类型的某些数据。

    数组中的数据如何解读取决于该列代表什么。
    对于该类型的命名空间nameSpace，列存储的是指向字符串堆（string heap）的索引，所以用列存储的值从字符串堆中获取字符串；
    有时直接使用列存储的值本身，例如MONO_ASSEMBLYREF_MAJOR_VERSION，
    如果你想获取程序集的主版本号，只需要 uint32_t majorVersion = cols[MONO_ASSEMBLYREF_MAJOR_VERSION];

    模块类型（The Module Type）
    打印出来的第一个类型叫 <Module>，这是 C# 编译器自动提供的一个类型，所有 C# DLL 和 EXE 都包含它。
    这个类型代表你的整个程序集。你的程序集至少会有一个模块Module，
    当然也有可能创建多文件程序集（Multifile Assembly），即包含多个模块的程序集。
    不过这些在本指南中并不重要，因为我们永远不会用到 <Module> 类

    加载应用程序集的类
    */
    void ScriptEngine::ReloadAssembly()
    {
        auto fullScriptModulePath = Project::GetAssetsAbsolutePath() / Project::GetActive()->GetConfig().scriptModulePath;
        if (!Project::GetActive()->GetConfig().scriptModulePath.empty() && std::filesystem::exists(fullScriptModulePath))
        {
            mono_domain_set(mono_get_root_domain(), false);
            mono_domain_unload(s_ScriptEngineData->AppDomain);
            LoadCoreAssembly(s_ScriptEngineData->CoreAssemblyFilepath);

            s_ScriptEngineData->CoreScriptClassMap.clear();

            for (auto& [nameSpace, className] : s_CoreAssamblyClassList)
            {
                LoadClass(nameSpace.c_str(), className.c_str(), true);
            }

            if (!LoadAppAssembly(fullScriptModulePath))
            {
                VOL_CORE_ERROR("[ScriptEngine] Could not load app assembly.");
                return;
            }

            s_ScriptEngineData->AppScriptClassMap.clear();

            const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_ScriptEngineData->AppAssemblyImage, MONO_TABLE_TYPEDEF);
            int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

            for (int32_t i = 0; i < numTypes; i++)
            {
                uint32_t cols[MONO_TYPEDEF_SIZE];
                mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

                const char* nameSpace = mono_metadata_string_heap(s_ScriptEngineData->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
                const char* name = mono_metadata_string_heap(s_ScriptEngineData->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);

                // 获取该类型对应的 MonoClass（注意：TypeDef 表的行索引从 1 开始，而 i 从 0 开始，因此传入 i + 1）
                uint32_t typeToken = mono_metadata_make_token(MONO_TABLE_TYPEDEF, i + 1);
                MonoClass* monoClass = mono_class_get(s_ScriptEngineData->AppAssemblyImage, typeToken);
                if (!monoClass)
                    continue;
                // 跳过枚举（Enum）
                if (mono_class_is_enum(monoClass))
                    continue;

                LoadClass(nameSpace, name, false);
            }
            ScriptGlue::RegisterComponents();

            // 重构脚本实例
            GetSceneContext();
        }
    }

    void ScriptEngine::OnRuntimeStart(Scene* scene)
    {
        s_ScriptEngineData->SceneContext = scene;
    }

    void ScriptEngine::OnRuntimeStop()
    {
        for (auto& [entityID, instance] : s_ScriptEngineData->EntityScriptInstanceMap)
        {
            ScriptEngine::EntityOnDisable(entityID);
        }
        for (auto& [entityID, instance] : s_ScriptEngineData->EntityScriptInstanceMap)
        {
            ScriptEngine::EntityOnDestroy(entityID);
        }

        s_ScriptEngineData->SceneContext = nullptr;
        s_ScriptEngineData->EntityScriptInstanceMap.clear();
        s_ScriptEngineData->EntityUpdateList = {};
        s_ScriptEngineData->EntityInvokeDelayedListBuffer.clear();
        s_ScriptEngineData->EntityInvokeDelayedList.clear();
    }

    Ref<ScriptClass> ScriptEngine::GetScriptClass(const std::string& fullClassName, bool isCore)
    {
        if (isCore)
        {
            auto it = s_ScriptEngineData->CoreScriptClassMap.find(fullClassName);
            if (it != s_ScriptEngineData->CoreScriptClassMap.end())
                return it->second;
        }
        else
        {
            auto it = s_ScriptEngineData->AppScriptClassMap.find(fullClassName);
            if (it != s_ScriptEngineData->AppScriptClassMap.end())
                return it->second;
        }
        return nullptr;
    }

    // 用于生成数组时获取元素类型，目前只有uint64_t类型
    MonoClass* ScriptEngine::GetScriptClass(ScriptFieldType type)
    {
        switch (type)
        {
        case ScriptFieldType::ULong:
            return mono_get_uint64_class();
        }
        return nullptr;
    }

    const std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetCoreScriptClassMap()
    {
        return s_ScriptEngineData->CoreScriptClassMap;
    }

    const std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetAppScriptClassMap()
    {
        return s_ScriptEngineData->AppScriptClassMap;
    }

    EntityScriptFieldMap& ScriptEngine::GetEntityScriptFieldMap()
    {
        return s_ScriptEngineData->EntityScriptFieldMap;
    }

    bool ScriptEngine::IsScriptClassLoaded(const std::string& fullClassName, bool isCore)
    {
        if (isCore)
            return s_ScriptEngineData->CoreScriptClassMap.find(fullClassName) != s_ScriptEngineData->CoreScriptClassMap.end();
        else
            return s_ScriptEngineData->AppScriptClassMap.find(fullClassName) != s_ScriptEngineData->AppScriptClassMap.end();
    }

    Ref<ScriptClass> ScriptEngine::GetScriptClassObject()
    {
        auto it = s_ScriptEngineData->CoreScriptClassMap.find("Volcano.Object");
        if (it != s_ScriptEngineData->CoreScriptClassMap.end())
            return it->second;
        else
            return Ref<ScriptClass>();
    }

    Ref<ScriptClass> ScriptEngine::GetScriptClassGameObject()
    {
        auto it = s_ScriptEngineData->CoreScriptClassMap.find("Volcano.GameObject");
        if (it != s_ScriptEngineData->CoreScriptClassMap.end())
            return it->second;
        else
            return Ref<ScriptClass>();
    }

    Ref<ScriptClass> ScriptEngine::GetScriptClassMonoBehaviour()
    {
        auto it = s_ScriptEngineData->CoreScriptClassMap.find("Volcano.MonoBehaviour");
        if (it != s_ScriptEngineData->CoreScriptClassMap.end())
            return it->second;
        else
            return Ref<ScriptClass>();
    }

    ScriptField* ScriptEngine::GetIDScriptField()
    {
        return s_ScriptEngineData->IDScriptField;
    }

    MonoImage* ScriptEngine::GetCoreAssemblyImage()
    {
        return s_ScriptEngineData->CoreAssemblyImage;
    }

    MonoImage* ScriptEngine::GetAppAssemblyImage()
    {
        return s_ScriptEngineData->AppAssemblyImage;
    }

    MonoDomain* ScriptEngine::GetCoreAssemblyDomain()
    {
        return s_ScriptEngineData->RootDomain;
    }

    MonoDomain* ScriptEngine::GetAppAssemblyDomain()
    {
        return s_ScriptEngineData->AppDomain;
    }

    Scene* ScriptEngine::GetSceneContext()
    {
        return s_ScriptEngineData->SceneContext;
    }

    Ref<ScriptInstanceMonoBehaviour> ScriptEngine::GetEntityScriptInstance(UUID entityID)
    {
        auto it = s_ScriptEngineData->EntityScriptInstanceMap.find(entityID);
        if (it == s_ScriptEngineData->EntityScriptInstanceMap.end())
            return nullptr;
        else
            return it->second;
    }

    std::unordered_map<UUID, Ref<ScriptInstanceMonoBehaviour>>& ScriptEngine::GetEntityScriptInstanceMap()
    {
        return s_ScriptEngineData->EntityScriptInstanceMap;
    }

    /*
    通过ScriptComponent中指定的脚本创建实体的脚本实例
    用途：
    1、挂载脚本时初始化实体的字段映射表EntityScriptFieldMap，此时不管EntityScriptInstanceMap
    2、进入运行状态时初始化实体的脚本实例表EntityScriptInstanceMap，用于运行中调用Update等，在OnRuntimeStop中清除
    
    判断脚本组件指定类名的脚本类已加载
    把现有的实体对应实例和字段销毁
    创建实例
    注入实体的脚本实例映射表
    注入实体的字段映射表（读取脚本类中字段的初始值）
    */
    Ref<ScriptInstanceMonoBehaviour> ScriptEngine::CreateMonoBehaviourScriptInstanceByEntity(Ref<Entity> entity, bool isRuntimeStart)
    {
        const auto& scriptComponent = entity->GetComponent<ScriptComponent>();
        if (ScriptEngine::IsScriptClassLoaded(scriptComponent.ClassName, false))
        {
            UUID entityID = entity->GetUUID();

            Ref<ScriptClass> scriptClass = GetScriptClass(scriptComponent.ClassName, false);
            Ref<ScriptInstanceMonoBehaviour> instance = 
                CreateRef<ScriptInstanceMonoBehaviour>(scriptClass, entityID, scriptComponent.enabled);
            
            if (isRuntimeStart)
            {
                s_ScriptEngineData->EntityScriptInstanceMap[entityID] = instance;
            }
            else
            {
                instance->InvokeReset();

                auto& fields = scriptClass->GetFields();

                for (const auto& [fieldName, scriptField] : fields)
                {
                    // 如果实体的字段映射表不存在该字段名，则新建字段并从实例中读取字段值
                    // 如果字段名存在的话，不改动，在切换挂载的脚本时，如果两个脚本间有同名字段，则会出错
                    ScriptFieldInstance* fieldInstance = s_ScriptEngineData->EntityScriptFieldMap.TryGetField(entityID, scriptClass->GetFullNameHash(), scriptField.nameHash);
                    if (fieldInstance == nullptr)
                    {
                        fieldInstance = s_ScriptEngineData->EntityScriptFieldMap.GetField(entityID, scriptClass->GetFullNameHash(), scriptField.nameHash);
                        instance->GetFieldValueInternal(scriptField.nameHash, fieldInstance->GetBufferPtr());
                    }
                }
            }

            return instance;
        }
        return nullptr;
    }

    template<typename Function>
    void ScriptEngine::EntityInvoke(UUID entityID, Function func)
    {
        auto it = s_ScriptEngineData->EntityScriptInstanceMap.find(entityID);
        if (it != s_ScriptEngineData->EntityScriptInstanceMap.end())
        {
            func(it->second);
        }
        else
        {
            VOL_CORE_ERROR("Could not find ScriptInstance for entity {}", (uint64_t)entityID);
        }
    }
    void ScriptEngine::EntityReset(UUID entityID)
    {
        EntityInvoke(entityID, [](Ref<ScriptInstanceMonoBehaviour> monoBehaviour) { monoBehaviour->InvokeReset(); });
    }

    void ScriptEngine::EntityAwake(UUID entityID)
    {
        EntityInvoke(entityID, [](Ref<ScriptInstanceMonoBehaviour> monoBehaviour) { monoBehaviour->InvokeAwake(); });
    }

    void ScriptEngine::EntityOnEnable(UUID entityID)
    {
        EntityInvoke(entityID, [](Ref<ScriptInstanceMonoBehaviour> monoBehaviour) { if (monoBehaviour->GetEnable()) { monoBehaviour->InvokeOnEnable(); } });
    }

    void ScriptEngine::EntityStart(UUID entityID)
    {
        EntityInvoke(entityID, [](Ref<ScriptInstanceMonoBehaviour> monoBehaviour) { if (monoBehaviour->GetEnable()) { monoBehaviour->InvokeStart(); } });
    }

    void ScriptEngine::EntityUpdate(UUID entityID)
    {
        EntityInvoke(entityID, [](Ref<ScriptInstanceMonoBehaviour> monoBehaviour) { if (monoBehaviour->GetEnable()) { monoBehaviour->InvokeUpdate(); } });
    }

    void ScriptEngine::EntityFixedUpdate(UUID entityID)
    {
        EntityInvoke(entityID, [](Ref<ScriptInstanceMonoBehaviour> monoBehaviour) { if (monoBehaviour->GetEnable()) { monoBehaviour->InvokeFixedUpdate(); } });
    }

    void ScriptEngine::EntityLateUpdate(UUID entityID)
    {
        EntityInvoke(entityID, [](Ref<ScriptInstanceMonoBehaviour> monoBehaviour) { if (monoBehaviour->GetEnable()) { monoBehaviour->InvokeLateUpdate(); } });
    }

    void ScriptEngine::EntityOnDisable(UUID entityID)
    {
        EntityInvoke(entityID, [](Ref<ScriptInstanceMonoBehaviour> monoBehaviour) { monoBehaviour->InvokeOnDisable(); });
    }

    void ScriptEngine::EntityOnDestroy(UUID entityID)
    {
        EntityInvoke(entityID, [](Ref<ScriptInstanceMonoBehaviour> monoBehaviour) { monoBehaviour->InvokeOnDestroy(); });
    }

    std::vector<InvokeDelayedData>& ScriptEngine::GetEntityInvokeDelayedListBuffer()
    {
        return s_ScriptEngineData->EntityInvokeDelayedListBuffer;
    }

    std::vector<InvokeDelayedData>& ScriptEngine::GetEntityInvokeDelayedList()
    {
        return s_ScriptEngineData->EntityInvokeDelayedList;
    }

    void ScriptEngine::RemoveEntityInvokeDelayed(UUID entityID, std::string methodName)
    {
        auto& listBuffer = s_ScriptEngineData->EntityInvokeDelayedListBuffer;
        for (auto it = listBuffer.begin(); it != listBuffer.end(); )
        {
            if (it->id == entityID && it->method.Name == methodName)
                it = listBuffer.erase(it);
            else
                it++;
        }

        auto& list = s_ScriptEngineData->EntityInvokeDelayedList;
        for (auto it = list.begin(); it != list.end(); )
        {
            if (it->id == entityID && it->method.Name == methodName)
                it = list.erase(it);
            else
                it++;
        }
    }

    void ScriptEngine::RemoveEntityInvokeDelayed(std::string methodName)
    {
        auto& listBuffer = s_ScriptEngineData->EntityInvokeDelayedListBuffer;
        for (auto it = listBuffer.begin(); it != listBuffer.end(); )
        {
            if (it->method.Name == methodName)
                it = listBuffer.erase(it);
            else
                it++;
        }

        auto& list = s_ScriptEngineData->EntityInvokeDelayedList;
        for (auto it = list.begin(); it != list.end(); )
        {
            if (it->method.Name == methodName)
                it = list.erase(it);
            else
                it++;
        }
    }

    void ScriptEngine::RemoveEntityInvokeDelayed(UUID entityID)
    {
        auto& listBuffer = s_ScriptEngineData->EntityInvokeDelayedListBuffer;
        for (auto it = listBuffer.begin(); it != listBuffer.end(); )
        {
            if (it->id == entityID)
                it = listBuffer.erase(it);
            else
                it++;
        }

        auto& list = s_ScriptEngineData->EntityInvokeDelayedList;
        for (auto it = list.begin(); it != list.end(); )
        {
            if (it->id == entityID)
                it = list.erase(it);
            else
                it++;
        }
    }

    std::queue<EntityUpdateBuffer>& ScriptEngine::GetEntityUpdateList()
    {
        return s_ScriptEngineData->EntityUpdateList;
    }

    /*
    告诉 Mono .NET 库的位置，相对于当前工作目录的相对路径

    启用调试支持：
    两个命令行参数，用于配置 Mono 的调试代理
    debugger-agent：指定调试通信方式（dt_socket 表示使用 TCP Socket），监听地址和端口（本地 127.0.0.1:2550），作为服务端等待调试器连接（server=y），不挂起启动（suspend=n），日志级别为 3（详细），日志输出到文件 MonoDebugger.log。
    --soft-breakpoints：启用软断点支持，允许在运行时设置断点

    mono_jit_parse_options：将上述参数解析并应用到 Mono JIT（即时编译器）引擎中，使调试选项生效
    mono_debug_init：初始化 Mono 的调试子系统，使用 Mono 原生的调试格式（MONO_DEBUG_FORMAT_MONO），允许与兼容的调试器（如 Visual Studio、MonoDevelop）交互。
    
    声明根域：
    mono_jit_init 启动 Mono JIT 运行时，并创建一个名为 "VolcanoJITRuntime" 的根应用程序域（AppDomain）。所有托管代码都将在此域中执行。
    mono_jit_init 会同时告诉 Mono 使用我们加载的第一个程序集所引用的运行时版本，也就是它会自动检测
    注：如果使用 mono_jit_init_version，可以精确指定想要的运行时版本

    如果调试已启用，告知调试子系统新域已创建，以便调试器能够正确跟踪该域内的代码执行

    将当前线程（即调用该函数的线程）设置为 Mono 运行时的主线程。
    这对于线程管理和垃圾回收的正确性很重要，因为 Mono 需要知道哪个线程是主线程，以处理线程挂起、同步等操作
    */
    void ScriptEngine::InitMono()
    {
        mono_set_assemblies_path("../Volcano/vendor/mono/lib");

        if (s_ScriptEngineData->EnabledDebugging)
        {
            const char* argv[2] = {
                "--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
                "--soft-breakpoints"
            };

            mono_jit_parse_options(2, (char**)argv);
            mono_debug_init(MONO_DEBUG_FORMAT_MONO);
        }

        s_ScriptEngineData->RootDomain = mono_jit_init("VolcanoJITRuntime");
        VOL_CORE_ASSERT(s_ScriptEngineData->RootDomain, "ScriptEngine::InitMono: rootDomain == nullptr");

        if (s_ScriptEngineData->EnabledDebugging)
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

    /*
    创建包装MonoClass的ScriptClass
    如果不是Core的类，检查ScriptClass是不是MonoBehaviour的子类，不是则结束读取
    在类库中加入ScriptClass
    读取Volcano::Entity类的ID字段
    遍历ScriptClass的字段，将public的字段读取到ScriptClass的字段库中
    遍历ScriptClass的方法，将方法读取到ScriptClass的方法库中
    */
    void ScriptEngine::LoadClass(const char* nameSpace, const char* className, bool isCore)
    {
        std::string fullName;
        if (strlen(nameSpace) != 0)
            fullName = fmt::format("{}.{}", nameSpace, className);
        else
            fullName = className;

        Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className, isCore);
        MonoClass* monoClass = scriptClass->GetMonoClass();
        if (isCore)
            s_ScriptEngineData->CoreScriptClassMap[fullName] = scriptClass;
        else if (mono_class_is_subclass_of(monoClass, ScriptEngine::GetScriptClassMonoBehaviour()->GetMonoClass(), false))
            s_ScriptEngineData->AppScriptClassMap[fullName] = scriptClass;
        else
            return;

        //MonoClassField* idField = mono_class_get_field_from_name(mono_class_from_name(ScriptEngine::GetCoreAssemblyImage(), "Volcano", "Object"), "ID");
        //ScriptFieldType idFieldType = Utils::MonoTypeToScriptFieldType(mono_field_get_type(idField));
        //scriptClass->m_Fields["ID"] = { idFieldType, "ID", idField };

        VOL_CORE_WARN("{}.{}:", nameSpace, className);
        VOL_CORE_WARN("  has {} fields:", mono_class_num_fields(monoClass));
        //VOL_CORE_WARN("    {} ({})", "ID", Utils::ScriptFieldTypeToString(idFieldType));


        void* fieldIterator = nullptr;
        while (MonoClassField* field = mono_class_get_fields(monoClass, &fieldIterator))
        {
            const char* fieldName = mono_field_get_name(field);

            uint32_t flags = mono_field_get_flags(field);
            if (flags & MONO_FIELD_ATTR_PUBLIC)
            {
                MonoType* type = mono_field_get_type(field);
                ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
                uint64_t fieldNameHadh = std::hash<std::string>{}(fieldName);
                scriptClass->m_Fields[fieldNameHadh] = { fieldType, fieldName, fieldNameHadh, field };
                if (fullName == "Volcano.Object" && fieldName == "ID")
                    s_ScriptEngineData->IDScriptField = &scriptClass->m_Fields[fieldNameHadh];
                VOL_CORE_WARN("    {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));
            }
        }

        int methodCount = mono_class_num_methods(monoClass);
        void* methodIterator = nullptr;

        VOL_CORE_WARN("  has {} methods:", methodCount);

        while (MonoMethod* method = mono_class_get_methods(monoClass, &methodIterator))
        {
            const char* methodName = mono_method_get_name(method);
            uint64_t methodNameHadh = std::hash<std::string>{}(methodName);
            scriptClass->m_Methods[methodName] = { methodName, methodNameHadh, method };

            VOL_CORE_WARN("    {}", methodName);
        }
    }

}