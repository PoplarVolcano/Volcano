#pragma once

#include "Volcano/Core/Timer.h"
#include "Volcano/Scene/Scene.h"
#include "Volcano/Scene/Entity.h"

#include <filesystem>
#include <string>
#include <map>
#include <queue>

// 如果不引入头文件，必须外部声明，但这些都是在c文件定义的结构，所以需要extern"C"
extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoType MonoType;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoClassField MonoClassField;
}

namespace Volcano {

	enum class ScriptFieldType
	{
		None = 0,
		Float, Double,
		Bool, Char, Byte, Short, Int, Long,
		UByte, UShort, UInt, ULong,
		String,
		Vector2, Vector3, Vector4,
		Quaternion, Matrix4x4,
		Entity, GameObject, Component, Transform, Behaviour, MonoBehaviour, Collider, Rigidbody
	};

	// C#字段：类型，字段名，字段数据
	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;

		MonoClassField* ClassField;
	};

	struct ScriptMethod
	{
		std::string Name;
		MonoMethod* ClassMethod;
	};

	class ScriptInstance;

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

	enum class EntityUpdateType
	{
		ADD, DESTROY, MOVE
	};

	struct EntityUpdateBuffer
	{
		EntityUpdateType type;
		UUID srcID = 0;
		UUID disID = 0;
	};

	// ScriptField + data storage
	struct ScriptFieldInstance
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
	private:
		uint8_t m_Buffer[64];

		friend class ScriptEngine;
		friend class ScriptInstance;
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;


	// C#类的封装类
	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);
		ScriptClass(MonoClass* monoClass, const std::string& classNamespace, const std::string& className, bool isCore);
		// 创建一个MonoClass类

		MonoObject* Instantiate();		// 创建一个由MonoClass类构成的mono对象并且初始化
		MonoMethod* GetMethod(const std::string& name, int parameterCount);                         // 获取类的函数
		ScriptMethod& GetMethod(const std::string& name);
		bool HasMethod(const std::string& name);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);// 调用类的函数
		MonoClass* GetClass() { return m_MonoClass; }

		const std::map<std::string, ScriptField>& GetFields() const { return m_Fields; }
		const std::map<std::string, ScriptMethod>& GetMethods() const { return m_Methods; }

		bool IsCore() { return m_IsCore; }
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		std::map<std::string, ScriptField> m_Fields;
		std::map<std::string, ScriptMethod> m_Methods;

		MonoClass* m_MonoClass = nullptr;

		bool m_IsCore;

		friend class ScriptEngine;
	};

	// C#类的实例，用于管理ScriptClass
	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity, bool enable = true);
		ScriptInstance(MonoObject* instance, bool enable = true);

		void InvokeAwake();
		void InvokeStart();
		void InvokeUpdate(float ts);
		void InvokeOnEnable();
		void InvokeOnDisable();
		void InvokeOnDestroy();
		void InvokeOnTriggerEnter(MonoObject* collider);
		void InvokeMethod(MonoMethod* method, void** params = nullptr);

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			bool success = GetFieldValueInternal(name, s_FieldValueBuffer);
			if (!success)
				return T();
			return *(T*)s_FieldValueBuffer;
		}

		template<typename T>
		void SetFieldValue(const std::string& name, T& value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			SetFieldValueInternal(name, &value);
		}
		MonoObject* GetManagedObject() { return m_Instance; }

		bool GetEnable() { return m_Enabled; }
		void SetEnabled(bool enabled) 
		{ 
			if (enabled != m_Enabled && enabled == true)
			{
				InvokeOnEnable();
			} 
			else if (enabled != m_Enabled && enabled == false)
			{
				InvokeOnDisable();
			}
			m_Enabled = enabled;
		}
		void ResetStart() { m_Start = false; }
	private:
		bool GetFieldValueInternal(const std::string& name, void* buffer);
		bool SetFieldValueInternal(const std::string& name, const void* value);
	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance        = nullptr;
		MonoMethod* m_Constructor     = nullptr;
		MonoMethod* m_AwakeMethod     = nullptr;
		MonoMethod* m_StartMethod     = nullptr;
		MonoMethod* m_UpdateMethod    = nullptr;
		MonoMethod* m_OnEnableMethod  = nullptr;
		MonoMethod* m_OnDisableMethod = nullptr;
		MonoMethod* m_OnDestroyMethod = nullptr;
		MonoMethod* m_OnTriggerEnterMethod = nullptr;

		inline static char s_FieldValueBuffer[16];

		bool m_Enabled = true;
		bool m_Start = false;

		friend class ScriptEngine;
		friend struct ScriptFieldInstance;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static bool LoadAssembly(const std::filesystem::path& filepath);

		static bool LoadAppAssembly(const std::filesystem::path& filepath);
		static void SetAppAssembly(const std::filesystem::path& filepath);
		static void ReloadAssembly();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static bool ClassExists(const std::string& fullClassName);
		static void CreateEntity(Entity entity);
		static void AwakeEntity(Entity entity);
		static void OnEnableEntity(Entity entity);
		static void StartEntity(Entity entity);
		static void UpdateEntity(Entity entity, Timestep ts);
		static void OnDisableEntity(Entity entity);
		static void OnDestroyEntity(Entity entity);

		static Scene* GetSceneContext();
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);
		static std::unordered_map<UUID, Ref<ScriptInstance>>& GetEntityScriptInstances();

		static Ref<ScriptClass> GetEntityClass();
		static Ref<ScriptClass> GetGameObjectClass();
		static Ref<ScriptClass> GetColliderClass();
		static Ref<ScriptClass> GetRigidbodyClass();
		static Ref<ScriptClass> GetMonoBehaviourClass();

		static Ref<ScriptClass> GetClass(const std::string& fullClassName);
		static std::unordered_map<std::string, Ref<ScriptClass>>& GetClasses();
		static ScriptFieldMap& GetScriptFieldMap(Entity entity);
		static std::vector<InvokeDelayedData>& GetEntityInvokeDelayedListBuffer();
		static std::vector<InvokeDelayedData>& GetEntityInvokeDelayedList();
		static void RemoveEntityInvokeDelayed(UUID entityID, std::string methodName);
		static void RemoveEntityInvokeDelayed(std::string methodName);
		static void RemoveEntityInvokeDelayed(UUID entityID);

		static std::queue<EntityUpdateBuffer>& GetEntityUpdateList();
		//static void SetScriptFieldMap(Entity entity, ScriptFieldMap scriptFieldMap);

		static MonoImage* GetCoreAssemblyImage();
		static MonoImage* GetAppAssemblyImage();
		//static MonoImage* GetMscorlibAssemblyImage();
		static MonoDomain* GetCoreAssemblyDomain();
		static MonoDomain* GetAppAssemblyDomain();

		static MonoObject* GetManagedInstance(UUID uuid);

		static MonoClass* GetClass(ScriptFieldType type);

		static MonoObject* CreateInstance(MonoClass* monoClass, UUID& entityID);

		static std::string MonoToString(MonoObject* monoObject);

		//static void ProcessMonoClass(MonoClass* monoClass);
	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);

		static void LoadAssamblyClass(const char* nameSpace, const char* className, bool isCore);
		//static void LoadAssemblyClasses();

		friend class ScriptClass;
		friend class ScriptGlue;
	};

	namespace Utils {

		inline const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
		{
			switch (fieldType)
			{
			case ScriptFieldType::None:          return "None";
			case ScriptFieldType::Float:         return "System.Float";
			case ScriptFieldType::Double:        return "System.Double";
			case ScriptFieldType::Bool:          return "System.Bool";
			case ScriptFieldType::Char:          return "System.Char";
			case ScriptFieldType::Byte:          return "System.Byte";
			case ScriptFieldType::Short:         return "System.Short";
			case ScriptFieldType::Int:           return "System.Int";
			case ScriptFieldType::Long:          return "System.Long";
			case ScriptFieldType::UByte:         return "System.UByte";
			case ScriptFieldType::UShort:        return "System.UShort";
			case ScriptFieldType::UInt:          return "System.UInt";
			case ScriptFieldType::ULong:         return "System.ULong";
			case ScriptFieldType::String:        return "System.String";
			case ScriptFieldType::Vector2:       return "Volcano.Vector2";
			case ScriptFieldType::Vector3:       return "Volcano.Vector3";
			case ScriptFieldType::Vector4:       return "Volcano.Vector4";
			case ScriptFieldType::Quaternion:    return "Volcano.Quaternion";
			case ScriptFieldType::Matrix4x4:     return "Volcano.Matrix4x4";
			case ScriptFieldType::Entity:        return "Volcano.Entity";
			case ScriptFieldType::GameObject:    return "Volcano.GameObject";
			case ScriptFieldType::Component:     return "Volcano.Component";
			case ScriptFieldType::Transform:     return "Volcano.Transform";
			case ScriptFieldType::Behaviour:     return "Volcano.Behaviour";
			case ScriptFieldType::MonoBehaviour: return "Volcano.MonoBehaviour";
			case ScriptFieldType::Collider:      return "Volcano.Collider";
			case ScriptFieldType::Rigidbody:     return "Volcano.Rigidbody";
			}
			VOL_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return "None";
		}

		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "None")                  return ScriptFieldType::None;
			if (fieldType == "System.Float")          return ScriptFieldType::Float;
			if (fieldType == "System.Double")         return ScriptFieldType::Double;
			if (fieldType == "System.Bool")           return ScriptFieldType::Bool;
			if (fieldType == "System.Char")           return ScriptFieldType::Char;
			if (fieldType == "System.Byte")           return ScriptFieldType::Byte;
			if (fieldType == "System.Short")          return ScriptFieldType::Short;
			if (fieldType == "System.Int")            return ScriptFieldType::Int;
			if (fieldType == "System.Long")           return ScriptFieldType::Long;
			if (fieldType == "System.UByte")          return ScriptFieldType::UByte;
			if (fieldType == "System.UShort")         return ScriptFieldType::UShort;
			if (fieldType == "System.UInt")           return ScriptFieldType::UInt;
			if (fieldType == "System.ULong")          return ScriptFieldType::ULong;
			if (fieldType == "System.String")         return ScriptFieldType::String;
			if (fieldType == "Volcano.Vector2")       return ScriptFieldType::Vector2;
			if (fieldType == "Volcano.Vector3")       return ScriptFieldType::Vector3;
			if (fieldType == "Volcano.Vector4")       return ScriptFieldType::Vector4;
			if (fieldType == "Volcano.Quaternion")    return ScriptFieldType::Quaternion;
			if (fieldType == "Volcano.Matrix4x4")     return ScriptFieldType::Matrix4x4;
			if (fieldType == "Volcano.Entity")        return ScriptFieldType::Entity;
			if (fieldType == "Volcano.GameObject")    return ScriptFieldType::GameObject;
			if (fieldType == "Volcano.Component")     return ScriptFieldType::Component;
			if (fieldType == "Volcano.Transform")     return ScriptFieldType::Transform;
			if (fieldType == "Volcano.Behaviour")     return ScriptFieldType::Behaviour;
			if (fieldType == "Volcano.MonoBehaviour") return ScriptFieldType::MonoBehaviour;
			if (fieldType == "Volcano.Collider")      return ScriptFieldType::Collider;
			if (fieldType == "Volcano.Rigidbody")     return ScriptFieldType::Rigidbody;

			//VOL_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return ScriptFieldType::None;
		}

	}
}