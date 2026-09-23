#include "volpch.h"
#include "ScriptInstance.h"

#include "Volcano/Scripting/ScriptEngine.h"
#include <mono/metadata/object.h>

namespace Volcano
{

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate();
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, uint64_t entityID)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate();
		MonoMethod* constructor = ScriptEngine::GetScriptClassObject()->GetMethod(".ctor", 1);
		if (constructor)
		{
			void* param = &entityID;
			GetScriptClass()->InvokeMethod(m_Instance, constructor, &param);
		}
	}

	ScriptInstance::ScriptInstance(MonoObject* instance)
	{
		MonoClass* monoClass = mono_object_get_class(instance);
		const char* nameSpace = mono_class_get_namespace(monoClass);
		const char* className = mono_class_get_name(monoClass);
		std::string fullName;
		if (strlen(nameSpace) != 0)
			fullName = fmt::format("{}.{}", nameSpace, className);
		else
			fullName = className;

		m_ScriptClass = ScriptEngine::GetScriptClass(fullName, false);
		if (m_ScriptClass == nullptr)
			m_ScriptClass = ScriptEngine::GetScriptClass(fullName, true);
		VOL_CORE_ASSERT(m_ScriptClass != nullptr);

		m_Instance = instance;
	}

	void ScriptInstance::InvokeMethod(MonoMethod* method, void** param) 
	{ 
		if (method)
		{
			m_ScriptClass->InvokeMethod(m_Instance, method, param);
		}
	}

	/*    
	C# 字段（field）和属性（property）的引用与交互。
	Mono 让我们可以（几乎）像操作普通字段一样操作属性——尽管属性本质上只是围绕一个字段和两个方法（getter/setter）的语法糖。
	和方法类似，我们不是从类实例中获取字段或属性，而是从类本身获取，然后通过类实例来访问它们。

	Mono 不关心我们想要访问的类、方法、字段或属性的可访问性（accessibility）。
	除非代码的作者明确允许，否则脚本引擎永远不应该能够设置非公共字段或属性的值。
	例如，你可以添加一个 C# 特性（attribute），用户可以把该特性添加到他们的私有字段上，告诉脚本引擎可以设置该字段的值。
	我们可以有一个名为 ShowInEditor 的特性，它允许引擎设置该字段的值。Unity 也有 SerializeField，作用相同。

	字段（Fields）

	你需要理解的最重要的一点是获取值类型（如 float、int 甚至 struct）和获取引用类型（如 class）之间的区别。

	如果你获取的是值类型（且该值没有被装箱，就像我们正在做的），你可以直接声明一个 C++ 等效类型的变量，
	然后传递该变量的内存地址。但 C++ 类型的大小必须与 C# 类型的大小匹配；如果你使用的是结构体（struct），其布局也必须匹配。

	如果你获取的是引用类型，则必须声明一个 MonoObject 指针，因为引用类型总是在堆上分配的。

	mono_field_get_value, 接受三个参数：
	我们要从中获取值的类实例；
	我们要获取值的字段；
	指向 C++ 端保存值的变量的指针。

	我们可以增加该值，并通过调用 mono_field_set_value，传入类实例、字段和变量的内存地址（假设它是值类型）来重新赋值给该字段。
	如果你想确认 C# 字段是否已正确更新，只需再次检索该值并检查即可。

	属性（Properties）
	mono_property_get_value，调用属性的 getter 方法。
	第一个参数是属性本身；
	第二个参数是类实例；
	第三个参数是 getter 方法可能期望的任何参数，该参数没有意义，因为属性的 getter 方法不能接受任何参数
	第四个参数是一个指向 MonoException* 的指针，我们可以从中获取方法抛出的异常。

	我们要获取的是一个字符串，因此我们得到一个指向 MonoString 结构的指针，它持有托管内存中 C# 字符串的指针。
	需要注意的是，mono_property_get_value 返回的是一个 MonoObject 指针，在字符串的情况下，应该直接将其强制转换为 MonoString 指针。
	如果是值类型（如 float），它会返回装箱在 MonoObject 中的值，这意味着我们需要拆箱（unbox）。

	现在我们有了字符串并修改了它，该把它赋值回 C# 了。
	我相信你已经意识到，这里我们进行了大量复制，但不用太担心，因为你很可能不会每帧都从 C++ 获取和设置 C# 值。

	mono_property_set_value，调用属性的 setter 方法。
	第一个参数是属性本身；
	第二个参数是类实例；
	第三个参数是值本身；
	第四个参数是一个指向 MonoException* 的指针，我们可以从中获取方法抛出的异常。
	注意，你不能像 mono_field_set 那样直接传递值类型的内存地址。如果你这样做，这个函数会崩溃。
	你需要创建一个 void* 数组，将地址存储在该数组中，然后传递数组。
	void* data[] = { &myValueTypeData };
	mono_property_set_value(prop, instance, data, nullptr);。

	对于 MonoObject* 或 MonoString*，可以直接传递内存地址并强制转换为 void**。
	不过我还是建议使用 void* 数组的方式，因为它始终有效。
	*/
	bool ScriptInstance::GetFieldValueInternal(uint64_t fieldNameHash, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(fieldNameHash);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		switch (field.type)
		{
		case ScriptFieldType::Object:
		case ScriptFieldType::GameObject:
		case ScriptFieldType::Component:
		case ScriptFieldType::Transform:
		case ScriptFieldType::Behaviour:
		case ScriptFieldType::MonoBehaviour:
		{
			// 读取引用类型字段
			MonoObject* targetObj = nullptr;
			mono_field_get_value(m_Instance, field.ClassField, &targetObj);
			if (targetObj == nullptr)
			{
				*(uint64_t*)buffer = 0;
			}
			else
			{
				// 将引用类型字段包装，然后获取ID字段
				// mono_class_get_field_from_name会递归查找继承链中的字段（包括父类），除了private字段
				MonoClassField* idField = mono_class_get_field_from_name(mono_object_get_class(targetObj), "ID");
				mono_field_get_value(targetObj, idField, buffer);
			}
			break;
		}
		case ScriptFieldType::Vector3:
		case ScriptFieldType::ULong:
		case ScriptFieldType::Float:
			mono_field_get_value(m_Instance, field.ClassField, buffer);
			break;
		}
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(uint64_t fieldNameHash, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto fieldIt = fields.find(fieldNameHash);
		if (fieldIt == fields.end())
			return false;

		const ScriptField& field = fieldIt->second;

		Ref<Entity> entity;
		auto& entityIDMap = ScriptEngine::GetSceneContext()->GetEntityIDMap();
		auto entityIDIt = entityIDMap.find(*(uint64_t*)value);
		switch (field.type)
		{
		case ScriptFieldType::Object:
		case ScriptFieldType::GameObject:
		case ScriptFieldType::Component:
		case ScriptFieldType::Transform:
		case ScriptFieldType::Behaviour:
		case ScriptFieldType::MonoBehaviour:
		{
			// 新建字段的引用类型实例并返回
			MonoClass* scriptFieldType = mono_class_from_mono_type(mono_field_get_type(field.ClassField));
			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(scriptFieldType, true);
			mono_field_set_value(m_Instance, field.ClassField, ScriptInstance(scriptClass, *(uint64_t*)value).GetManagedInstance());
			break;
		}
		case ScriptFieldType::Vector3:
		case ScriptFieldType::ULong:
			mono_field_set_value(m_Instance, field.ClassField, (void*)value);
			break;
		case ScriptFieldType::Float:
			mono_field_set_value(m_Instance, field.ClassField, (void*)value);
			break;
		}
		return true;
	}

	ScriptInstanceMonoBehaviour::ScriptInstanceMonoBehaviour(Ref<ScriptClass> scriptClass, UUID entityID, bool enable)
		: ScriptInstance(scriptClass), m_Enabled(enable)
	{
		m_Constructor       = ScriptEngine::GetScriptClassObject()->GetMethod(".ctor", 1);
		m_ResetMethod       = scriptClass->GetMethod("Reset", 0); ;
		m_AwakeMethod       = scriptClass->GetMethod("Awake", 0); ;
		m_OnEnableMethod    = scriptClass->GetMethod("OnEnable", 0); ;
		m_StartMethod       = scriptClass->GetMethod("Start", 0); ;
		m_UpdateMethod      = scriptClass->GetMethod("Update", 0); ;
		m_FixedUpdateMethod = scriptClass->GetMethod("FixedUpdate", 0); ;
		m_LateUpdateMethod  = scriptClass->GetMethod("LateUpdate", 0); ;
		m_OnDisableMethod   = scriptClass->GetMethod("OnDisable", 0); ;
		m_OnDestroyMethod   = scriptClass->GetMethod("OnDestroy", 0); ;

		if (m_Constructor)
		{
			void* param = &entityID;
			GetScriptClass()->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	ScriptInstanceMonoBehaviour::ScriptInstanceMonoBehaviour(MonoObject* instance, bool enable)
		: ScriptInstance(instance), m_Enabled(enable)
	{
		m_Constructor = ScriptEngine::GetScriptClassObject()->GetMethod(".ctor", 1);	// 获取C#Object类的有一个参数的构造函数
	}

	void ScriptInstanceMonoBehaviour::InvokeReset()       { InvokeMethod(m_ResetMethod); }
	void ScriptInstanceMonoBehaviour::InvokeAwake()       { InvokeMethod(m_AwakeMethod); }
	void ScriptInstanceMonoBehaviour::InvokeStart()       { if (!m_Start) { InvokeMethod(m_StartMethod); m_Start = true; } }
	void ScriptInstanceMonoBehaviour::InvokeUpdate()      { InvokeMethod(m_UpdateMethod); }
	void ScriptInstanceMonoBehaviour::InvokeFixedUpdate() { InvokeMethod(m_FixedUpdateMethod); }
	void ScriptInstanceMonoBehaviour::InvokeLateUpdate()  { InvokeMethod(m_LateUpdateMethod); }
	void ScriptInstanceMonoBehaviour::InvokeOnEnable()    { InvokeMethod(m_OnEnableMethod); }
	void ScriptInstanceMonoBehaviour::InvokeOnDisable()   { InvokeMethod(m_OnDisableMethod); }
	void ScriptInstanceMonoBehaviour::InvokeOnDestroy()   { InvokeMethod(m_OnDestroyMethod); }
	
}