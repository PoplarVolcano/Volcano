#pragma once

#include "Volcano/Core/Timer.h"
#include "Volcano/Scene/Scene.h"
#include "Volcano/Scene/Entity.h"

#include <filesystem>
#include <string>
#include <map>

// 如果不引入头文件，必须外部声明，但这些都是在c文件定义的结构，所以需要extern"C"
extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
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
		Entity
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
		bool firstInvoke = false;
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
		ScriptClass(MonoClass* monoClass, const std::string& classNamespace, const std::string& className);
		// 创建一个MonoClass类

		MonoObject* Instantiate();		// 创建一个由MonoClass类构成的mono对象并且初始化
		MonoMethod* GetMethod(const std::string& name, int parameterCount);                         // 获取类的函数
		ScriptMethod& GetMethod(const std::string& name);
		bool HasMethod(const std::string& name);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);// 调用类的函数
		MonoClass* GetClass() { return m_MonoClass; }

		const std::map<std::string, ScriptField>& GetFields() const { return m_Fields; }
		const std::map<std::string, ScriptMethod>& GetMethods() const { return m_Methods; }
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		std::map<std::string, ScriptField> m_Fields;
		std::map<std::string, ScriptMethod> m_Methods;

		MonoClass* m_MonoClass = nullptr;

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

		bool GetEnable() { return m_Enable; }
		void SetEnable(bool enable) 
		{ 
			if (enable != m_Enable && enable == true)
			{
				InvokeOnEnable();
			} 
			else if (enable != m_Enable && enable == false)
			{
				InvokeOnDisable();
			}
			m_Enable = enable;
		}
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

		inline static char s_FieldValueBuffer[16];

		bool m_Enable = true;

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

		static bool EntityClassExists(const std::string& fullClassName);
		static void CreateEntity(Entity entity, bool enable = true);
		static void AwakeEntity(Entity entity, Timestep ts);
		static void OnEnableEntity(Entity entity, Timestep ts);
		static void StartEntity(Entity entity, Timestep ts);
		static void UpdateEntity(Entity entity, Timestep ts);
		static void OnDisableEntity(Entity entity, Timestep ts);

		static Scene* GetSceneContext();
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

		static Ref<ScriptClass> GetEntityClass();

		static Ref<ScriptClass> GetEntityClass(const std::string& name);
		static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();
		static ScriptFieldMap& GetScriptFieldMap(Entity entity);
		static std::vector<InvokeDelayedData>& GetEntityInvokeDelayedList();
		//static void SetScriptFieldMap(Entity entity, ScriptFieldMap scriptFieldMap);

		static MonoImage* GetCoreAssemblyImage();

		static MonoImage* GetAppAssemblyImage();

		//static MonoImage* GetMscorlibAssemblyImage();

		static MonoDomain* GetCoreAssemblyDomain();

		static MonoDomain* GetAppAssemblyDomain();

		static MonoObject* GetManagedInstance(UUID uuid);

		static MonoClass* GetClass(ScriptFieldType type);

		static void ProcessMonoClass(MonoClass* monoClass);
	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);

		static void LoadAssemblyClasses();

		friend class ScriptClass;
		friend class ScriptGlue;
	};

	namespace Utils {

		inline const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
		{
			switch (fieldType)
			{
			case ScriptFieldType::None:       return "None";
			case ScriptFieldType::Float:      return "Float";
			case ScriptFieldType::Double:     return "Double";
			case ScriptFieldType::Bool:       return "Bool";
			case ScriptFieldType::Char:       return "Char";
			case ScriptFieldType::Byte:       return "Byte";
			case ScriptFieldType::Short:      return "Short";
			case ScriptFieldType::Int:        return "Int";
			case ScriptFieldType::Long:       return "Long";
			case ScriptFieldType::UByte:      return "UByte";
			case ScriptFieldType::UShort:     return "UShort";
			case ScriptFieldType::UInt:       return "UInt";
			case ScriptFieldType::ULong:      return "ULong";
			case ScriptFieldType::String:     return "String";
			case ScriptFieldType::Vector2:    return "Vector2";
			case ScriptFieldType::Vector3:    return "Vector3";
			case ScriptFieldType::Vector4:    return "Vector4";
			case ScriptFieldType::Quaternion: return "Quaternion";
			case ScriptFieldType::Matrix4x4:  return "Matrix4x4";
			case ScriptFieldType::Entity:     return "Entity";
			}
			VOL_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return "None";
		}

		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "None")       return ScriptFieldType::None;
			if (fieldType == "Float")      return ScriptFieldType::Float;
			if (fieldType == "Double")     return ScriptFieldType::Double;
			if (fieldType == "Bool")       return ScriptFieldType::Bool;
			if (fieldType == "Char")       return ScriptFieldType::Char;
			if (fieldType == "Byte")       return ScriptFieldType::Byte;
			if (fieldType == "Short")      return ScriptFieldType::Short;
			if (fieldType == "Int")        return ScriptFieldType::Int;
			if (fieldType == "Long")       return ScriptFieldType::Long;
			if (fieldType == "UByte")      return ScriptFieldType::UByte;
			if (fieldType == "UShort")     return ScriptFieldType::UShort;
			if (fieldType == "UInt")       return ScriptFieldType::UInt;
			if (fieldType == "ULong")      return ScriptFieldType::ULong;
			if (fieldType == "String")     return ScriptFieldType::String;
			if (fieldType == "Vector2")    return ScriptFieldType::Vector2;
			if (fieldType == "Vector3")    return ScriptFieldType::Vector3;
			if (fieldType == "Vector4")    return ScriptFieldType::Vector4;
			if (fieldType == "Quaternion") return ScriptFieldType::Quaternion;
			if (fieldType == "Matrix4x4")  return ScriptFieldType::Matrix4x4;
			if (fieldType == "Entity")     return ScriptFieldType::Entity;

			VOL_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return ScriptFieldType::None;
		}

	}
}