#include "volpch.h"
#include "ScriptClass.h"
#include "Volcano/Scripting/ScriptEngine.h"

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

namespace Volcano
{
    /*
    实例化一个类涉及两个步骤：分配对象内存和调用正确的构造函数。

    注意，分配类和构造类是两件截然不同的事：
    分配仅仅意味着分配足够的内存来容纳类的所有数据。
    构造意味着调用类的某个构造函数，这会初始化类中存储的所有字段和属性。

    注：在调用构造函数之前，类实例MonoObject是无效的

    分配完实例后，需要调用构造函数（即初始化实例）。
    类可以有多个构造函数，应该调用哪一个？
    要么事先知道每个类有哪些构造函数，要么强制要求——所有可以从 C++ 构造的类都必须有无参构造函数。
    我们两种方式都用。对于大多数类，我们假设它们有无参构造函数，但我们也支持带任意数量参数的构造函数。
    
    mono_class_from_name：通过名称在指定的程序集镜像（MonoImage）中查找并获取一个托管类（MonoClass）的指针
    mono_object_new ：在托管堆（Managed Heap）上创建一个新的托管对象实例instance的核心函数
    mono_runtime_object_init：调用类实例instance的无参（默认）构造函数 
    */
    ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
        : m_ClassNamespace(classNamespace),
        m_ClassName(className),
        m_FullNameHash(std::hash<std::string>{}(fmt::format("{}.{}", m_ClassNamespace, m_ClassName))),
        m_IsCore(isCore)
    {
        m_MonoClass = mono_class_from_name(isCore ? ScriptEngine::GetCoreAssemblyImage() : ScriptEngine::GetAppAssemblyImage(), classNamespace.c_str(), className.c_str());
    }


    ScriptClass::ScriptClass(MonoClass* monoClass, bool isCore)
        : m_ClassNamespace(mono_class_get_namespace(monoClass)),
        m_ClassName(mono_class_get_name(monoClass)),
        m_FullNameHash(std::hash<std::string>{}(fmt::format("{}.{}", m_ClassNamespace, m_ClassName))),
        m_MonoClass(monoClass),
        m_IsCore(isCore)
    {}

    MonoObject* ScriptClass::Instantiate()
    {
        MonoObject* instance = mono_object_new(ScriptEngine::GetAppAssemblyDomain(), m_MonoClass);
        mono_runtime_object_init(instance);
        return instance;
    }


    /*
    Mono提供了两种调用 C# 方法的方式：mono_runtime_invoke 和 Unmanaged to Managed Thunks（非托管到托管的 thunk）

    使用 mono_runtime_invoke 相比 Unmanaged Method Thunks 要慢一些，但它可以调用任何方法并传入任何参数，
    还会对传入的对象和参数做更多的错误检查和验证。

    Unmanaged to Managed Thunks（非托管到托管的 thunk）是 Mono 版本 2 中引入的概念，
    它允许以比 mono_runtime_invoke 低得多的开销调用 C# 方法。

    如果每秒要调用某个 C# 方法很多次——比如你有一个 C# 的 OnUpdate 方法，
    每秒调用 60~144 次——那么应该使用 Unmanaged to Managed Thunk。

    Unmanaged to Managed Thunk 会为代码的调用创建一个自定义的调用方法（比如一个自定义的 "trampoline"），
    而且这个调用方法是针对你给出的方法签名专门生成的，也就是说，可以传入哪些参数没有任何歧义。

    如果编译时不知道方法签名，或者只是偶尔调用该方法（而不是每秒多次），应该使用 mono_runtime_invoke。
    如果每秒要调用某个 C# 方法多次（超过 10 次就算），而且编译时知道该方法的签名，应该使用Unmanaged to Managed Thunks


    获取 C# 方法引用的方式：
    一种是你正在解析 C# 程序集，事先不知道里面有哪些方法；
    另一种是你在加载程序集之前，就已经知道了方法名、签名以及它属于哪个类。

    CallPrintFloatVarMethod 会在 ScriptTest 类的实例上调用（invoke）PrintFloatVar 方法。
    记住，方法存储在类中，但你是在类的实例上调用它们的。
    所有 C# 方法都有一个隐式参数，指向调用该方法的类实例——this 关键字。

    mono_object_get_class，从类实例中获取C#类（MonoClass），需传入 MonoObject 指针作为唯一参数。

    mono_class_get_method_from_name，获取 C# 方法（MonoMethod）。
    第一个参数是方法所属的类。如果方法实际上不存在于该类中，该函数会返回 nullptr。
    第二个参数是要获取的方法名。
    第三个参数是该方法有多少个参数。如果没有参数，直接传 0；也可以传 -1，那样 Mono 会直接返回它找到的第一个版本的方法。
    注：如果存在多个同名且参数数量相同的方法版本，这个函数就无法正常工作了，因为它只检查参数数量，而不检查实际的方法签名。

    mono_runtime_invoke，调用C#方法
    第一个参数：要调用的 C# 方法的指针。
    第二个参数：调用该方法的类实例。
    第三个参数：指向要传入的参数数组的指针。PrintFloatVar 没有参数，直接传 nullptr。
    第四个参数：指向exception的指针。如果暂时不关心异常，可以直接传 nullptr。

    调用 mono_runtime_invoke 前先声明一个 MonoObject 的指针，命名为 exception，并初始化为 nullptr。
    如果调用的方法抛出异常，脚本引擎需要知道这件事，这样才能把异常记录到控制台窗口或类似的地方。
    如果方法抛出异常，mono_runtime_invoke 会用异常实例填充那个 MonoObject，然后我们就可以用它来获取出错信息了。

    mono_runtime_invoke 可以返回一个 MonoObject*。
    如果调用的方法有返回值，并且想在 C++ 中获取并处理该返回值，这会非常有用。
    如果方法标记为 void，mono_runtime_invoke 会直接返回 nullptr。

    向方法传递参数：
    传递参数通常涉及在非托管内存和托管内存之间“封送（Marshalling）”数据。
    封送：(https://mark-borg.github.io/blog/2017/interop/)
    Mono 几乎从不帮我们自动处理封送，这意味着我们之后需要做一些手动的类型检查和转换。

    */
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

}