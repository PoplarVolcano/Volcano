#pragma once

#include "Volcano/Core/Core.h"

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

	enum class VOL_API ScriptFieldType
	{
		None = 0,
		Float, Double,
		Bool, Char, Byte, Short, Int, Long,
		UByte, UShort, UInt, ULong,
		String,
		Vector2, Vector3, Vector4,
		Quaternion, Matrix4x4,
		Object, GameObject, ScriptableObject, Component, Transform, Behaviour, MonoBehaviour
	};

	// C#字段：类型，字段名，字段数据
	struct VOL_API ScriptField
	{
		ScriptFieldType type;
		std::string name;
		uint64_t nameHash;

		MonoClassField* ClassField;
	};

	struct ScriptMethod
	{
		std::string Name;
		uint64_t NameHash;
		MonoMethod* ClassMethod;
	};

	class VOL_API ScriptClass
	{
	public:
		ScriptClass() = default;
		// 获取classNamespace.className的MonoClass的包装
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);
		// 构造monoClass的包装
		ScriptClass(MonoClass* monoClass, bool isCore);

		// 默认构造
		MonoObject* Instantiate();

		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		ScriptMethod& GetMethod(const std::string& name);
		bool HasMethod(const std::string& name);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
		
		uint64_t GetFullNameHash() const { return m_FullNameHash; }
		MonoClass* GetMonoClass() { return m_MonoClass; }
		const std::map<uint64_t, ScriptField>& GetFields() const { return m_Fields; }
		const std::map<std::string, ScriptMethod>& GetMethods() const { return m_Methods; }

		bool IsCore() const { return m_IsCore; }
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;
		uint64_t m_FullNameHash;
		MonoClass* m_MonoClass = nullptr;
		bool m_IsCore;
		std::map<uint64_t, ScriptField> m_Fields;
		std::map<std::string, ScriptMethod> m_Methods;

		friend class ScriptEngine;
	};

}