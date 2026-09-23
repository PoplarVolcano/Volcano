#pragma once

#include "Volcano/Scripting/ScriptInstance.h"
#include "Volcano/Scene/Scene.h"
#include "Volcano/Core/Timer.h"

namespace Volcano
{
	// ScriptField + data storage
	struct VOL_API ScriptFieldInstance
	{
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer));
		}

		template<typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 64, "Type too large!");
			return *(T*)m_Buffer;
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 64, "Type too large!");
			memcpy(m_Buffer, &value, sizeof(T));
		}

		void* GetBuffer() { return m_Buffer; }
		void** GetBufferPtr() { return (void**)&m_Buffer; }
	private:
		uint8_t m_Buffer[64];

		friend class ScriptEngine;
		friend class ScriptInstance;
	};

	/*
	可访问性存储在一个位域（bit field）
	因为 C# 允许将类字段和属性标记为 protected internal 或 private protected。
	如果你只关心字段/属性是否是公共的，你也可以直接返回一个 bool，并给函数起名为 IsFieldPublic / IsPropertyPublic。
	*/
	enum class Accessibility : uint8_t
	{
		None = 0,
		Private = (1 << 0),
		Internal = (1 << 1),
		Protected = (1 << 2),
		Public = (1 << 3)
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

	using ClassScriptFieldMap = std::unordered_map<std::string, ScriptFieldMap>;
	// 延迟调用
	/*
	调用对象的id
	调用对象的实例
	调用的方法
	加入延迟列表后多久执行
	重复率
	*/
	struct InvokeDelayedData
	{
		UUID id;
		Ref<ScriptInstance> instance;
		ScriptMethod method;
		float time;
		float repeatRate;
		Timer timer;
		bool firstInvoke = true;
	};

	// 实体更新类型
	enum class EntityUpdateType
	{
		ADD, DESTROY, MOVE
	};

	// 实体更新缓冲
	struct EntityUpdateBuffer
	{
		EntityUpdateType type;
		UUID srcID = 0;
		UUID disID = 0;
	};

	struct FieldKey {
		uint64_t EntityID;
		uint64_t ClassNameHash;
		uint64_t FieldNameHash;

		bool operator==(const FieldKey& other) const {
			return EntityID == other.EntityID &&
				ClassNameHash == other.ClassNameHash &&
				FieldNameHash == other.FieldNameHash;
		}
	};

	struct FieldKeyHash
	{
		size_t operator()(const FieldKey& k) const {
			uint64_t h = k.EntityID;
			h ^= (uint64_t)k.ClassNameHash + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= (uint64_t)k.FieldNameHash + 0x9e3779b9 + (h << 6) + (h >> 2);
			return (size_t)h;
		}
	};

	// 实体和脚本字段的映射表
	// TODO：现在在ScriptEngineData中作为全局资源使用，引擎支持多场景之后应该在每一个场景中单独声明和使用，以免ID冲突
	struct VOL_API EntityScriptFieldMap
	{
		// std::unordered_map 的第三个模板参数是哈希器类型，默认是 std::hash<FieldKey>。
		std::unordered_map<FieldKey, ScriptFieldInstance, FieldKeyHash> Map;

		bool IsFieldExist(uint64_t entityID, uint64_t classNameHash, uint64_t fieldNameHash)
		{
			FieldKey key = { entityID, classNameHash, fieldNameHash };
			return Map.find(key) != Map.end();
		}
		bool IsFieldExist(uint64_t entityID)
		{
			for (const auto& pair : Map)
			{
				if (pair.first.EntityID == entityID)
					return true;
			}
			return false;
		}
		void SetField(uint64_t entityID, uint64_t classNameHash, uint64_t fieldNameHash, const ScriptFieldInstance& ScriptFieldInstance)
		{
			FieldKey key = { entityID, classNameHash, fieldNameHash };
			Map[key] = ScriptFieldInstance;
		}
		ScriptFieldInstance* GetField(uint64_t entityID, uint64_t classNameHash, uint64_t fieldNameHash)
		{
			FieldKey key = { entityID, classNameHash, fieldNameHash };
			return &Map[key];

		}
		ScriptFieldInstance* TryGetField(uint64_t entityID, uint64_t classNameHash, uint64_t fieldNameHash)
		{
			FieldKey key = { entityID, classNameHash, fieldNameHash };
			auto it = Map.find(key);
			if (it != Map.end())
				return &it->second;
			else
				return nullptr;
		}
	};

	class VOL_API ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static bool LoadCoreAssembly(const std::filesystem::path& filepath);
		static bool LoadAppAssembly(const std::filesystem::path& filepath);
		static void ReloadAssembly();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static Ref<ScriptClass> GetScriptClass(const std::string& fullClassName, bool isCore);
		static MonoClass* GetScriptClass(ScriptFieldType type);
		static const std::unordered_map<std::string, Ref<ScriptClass>>& GetCoreScriptClassMap();
		static const std::unordered_map<std::string, Ref<ScriptClass>>& GetAppScriptClassMap();
		static EntityScriptFieldMap& GetEntityScriptFieldMap();

		static bool IsScriptClassLoaded(const std::string& fullClassName, bool isCore);
		static Ref<ScriptClass> GetScriptClassObject();
		static Ref<ScriptClass> GetScriptClassGameObject();
		static Ref<ScriptClass> GetScriptClassMonoBehaviour();
		static ScriptField* GetIDScriptField();

		static MonoImage* GetCoreAssemblyImage();
		static MonoImage* GetAppAssemblyImage();
		static MonoDomain* GetCoreAssemblyDomain();
		static MonoDomain* GetAppAssemblyDomain();

		static Scene* GetSceneContext();
		static Ref<ScriptInstanceMonoBehaviour> GetEntityScriptInstance(UUID entityID);  // 通过ID获取entity的脚本实例GameObject
		static std::unordered_map<UUID, Ref<ScriptInstanceMonoBehaviour>>& GetEntityScriptInstanceMap(); // 获取entity的脚本实例GameObject的库

		/*
        通过ScriptComponent中指定的脚本创建实体的脚本实例
        用途：
        1、挂载脚本时初始化实体的字段映射表EntityScriptFieldMap，此时不管EntityScriptInstanceMap
        2、进入运行状态时初始化实体的脚本实例表EntityScriptInstanceMap，用于运行中调用Update等，在OnRuntimeStop中清除
    
		创建实体的脚本实例

		判断脚本组件指定的类名已加载
		把现有的实体对应实例和字段销毁
		创建实例
		注入实体的脚本实例映射表
		注入实体的字段映射表（读取脚本类中字段的初始值）

		回调	    编辑器	        运行时
        Reset	    √ 添加组件时	×
        OnValidate	√ 字段改变时	×
        Awake	    ×	            √ 实例注册后一次
        OnEnable	×	            √ 每次启用
        Start	    ×	            √ 首次 Update 前
        OnDisable	×	            √ 每次禁用/移除
        OnDestroy	×	            √ 销毁时
		*/
		static Ref<ScriptInstanceMonoBehaviour> CreateMonoBehaviourScriptInstanceByEntity(Ref<Entity> entity, bool isRuntimeStart);
		template<typename Function>
		static void EntityInvoke(UUID entityID, Function func);
		static void EntityReset(UUID entityID);
		static void EntityAwake(UUID entityID);
		static void EntityOnEnable(UUID entityID);
		static void EntityStart(UUID entityID);
		static void EntityUpdate(UUID entityID);
		static void EntityFixedUpdate(UUID entityID);
		static void EntityLateUpdate(UUID entityID);
		static void EntityOnDisable(UUID entityID);
		static void EntityOnDestroy(UUID entityID);

		static std::vector<InvokeDelayedData>& GetEntityInvokeDelayedListBuffer();
		static std::vector<InvokeDelayedData>& GetEntityInvokeDelayedList();
		static void RemoveEntityInvokeDelayed(UUID entityID, std::string methodName);
		static void RemoveEntityInvokeDelayed(std::string methodName);
		static void RemoveEntityInvokeDelayed(UUID entityID);
		static std::queue<EntityUpdateBuffer>& GetEntityUpdateList();

	private:
		// 初始化Mono，创建根域RootDomin
		static void InitMono();
		static void ShutdownMono();
		static void LoadClass(const char* nameSpace, const char* className, bool isCore);
	};


	namespace Utils
	{
		inline const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
		{
			switch (fieldType)
			{
			case ScriptFieldType::None:             return "None";
			case ScriptFieldType::Float:            return "System.Float";
			case ScriptFieldType::Double:           return "System.Double";
			case ScriptFieldType::Bool:             return "System.Bool";
			case ScriptFieldType::Char:             return "System.Char";
			case ScriptFieldType::Byte:             return "System.Byte";
			case ScriptFieldType::Short:            return "System.Short";
			case ScriptFieldType::Int:              return "System.Int";
			case ScriptFieldType::Long:             return "System.Long";
			case ScriptFieldType::UByte:            return "System.UByte";
			case ScriptFieldType::UShort:           return "System.UShort";
			case ScriptFieldType::UInt:             return "System.UInt";
			case ScriptFieldType::ULong:            return "System.ULong";
			case ScriptFieldType::String:           return "System.String";
			case ScriptFieldType::Vector2:          return "Volcano.Vector2";
			case ScriptFieldType::Vector3:          return "Volcano.Vector3";
			case ScriptFieldType::Vector4:          return "Volcano.Vector4";
			case ScriptFieldType::Quaternion:       return "Volcano.Quaternion";
			case ScriptFieldType::Matrix4x4:        return "Volcano.Matrix4x4";
			case ScriptFieldType::Object:           return "Volcano.Object";
			case ScriptFieldType::GameObject:       return "Volcano.GameObject";
			case ScriptFieldType::ScriptableObject: return "Volcano.ScriptableObject";
			case ScriptFieldType::Component:        return "Volcano.Component";
			case ScriptFieldType::Transform:        return "Volcano.Transform";
			case ScriptFieldType::Behaviour:        return "Volcano.Behaviour";
			case ScriptFieldType::MonoBehaviour:    return "Volcano.MonoBehaviour";
			}
			VOL_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return "None";
		}

		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "None")                     return ScriptFieldType::None;
			if (fieldType == "System.Float")             return ScriptFieldType::Float;
			if (fieldType == "System.Double")            return ScriptFieldType::Double;
			if (fieldType == "System.Bool")              return ScriptFieldType::Bool;
			if (fieldType == "System.Char")              return ScriptFieldType::Char;
			if (fieldType == "System.Byte")              return ScriptFieldType::Byte;
			if (fieldType == "System.Short")             return ScriptFieldType::Short;
			if (fieldType == "System.Int")               return ScriptFieldType::Int;
			if (fieldType == "System.Long")              return ScriptFieldType::Long;
			if (fieldType == "System.UByte")             return ScriptFieldType::UByte;
			if (fieldType == "System.UShort")            return ScriptFieldType::UShort;
			if (fieldType == "System.UInt")              return ScriptFieldType::UInt;
			if (fieldType == "System.ULong")             return ScriptFieldType::ULong;
			if (fieldType == "System.String")            return ScriptFieldType::String;
			if (fieldType == "Volcano.Vector2")          return ScriptFieldType::Vector2;
			if (fieldType == "Volcano.Vector3")          return ScriptFieldType::Vector3;
			if (fieldType == "Volcano.Vector4")          return ScriptFieldType::Vector4;
			if (fieldType == "Volcano.Quaternion")       return ScriptFieldType::Quaternion;
			if (fieldType == "Volcano.Matrix4x4")        return ScriptFieldType::Matrix4x4;
			if (fieldType == "Volcano.Object")           return ScriptFieldType::Object;
			if (fieldType == "Volcano.GameObject")       return ScriptFieldType::GameObject;
			if (fieldType == "Volcano.ScriptableObject") return ScriptFieldType::ScriptableObject;
			if (fieldType == "Volcano.Component")        return ScriptFieldType::Component;
			if (fieldType == "Volcano.Transform")        return ScriptFieldType::Transform;
			if (fieldType == "Volcano.Behaviour")        return ScriptFieldType::Behaviour;
			if (fieldType == "Volcano.MonoBehaviour")    return ScriptFieldType::MonoBehaviour;

			//VOL_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return ScriptFieldType::None;
		}

	}
}