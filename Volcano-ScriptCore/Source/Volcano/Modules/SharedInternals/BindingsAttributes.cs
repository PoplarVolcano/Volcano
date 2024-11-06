using System;

namespace Volcano.Bindings
{
    // From Unity VisibleToOtherModulesAttribute
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Constructor | AttributeTargets.Delegate | AttributeTargets.Enum | AttributeTargets.Field | AttributeTargets.Interface | AttributeTargets.Method | AttributeTargets.Property | AttributeTargets.Struct, Inherited = false)]
    [VisibleToOtherModules]
    internal class VisibleToOtherModulesAttribute : Attribute
    {
        // This attributes controls visibility of internal types and members to other modules.
        // 此属性控制内部类型和成员对其他模块的可见性。
        public VisibleToOtherModulesAttribute()
        {
        }

        public VisibleToOtherModulesAttribute(params string[] modules)
        {
        }
    }

    // 绑定
    interface IBindingsAttribute
    {
    }

    // 名字提供者
    interface IBindingsNameProviderAttribute : IBindingsAttribute
    {
        string Name { get; set; }
    }

    // 头文件提供者
    interface IBindingsHeaderProviderAttribute : IBindingsAttribute
    {
        string Header { get; set; }
    }

    // 是否线程安全提供者
    interface IBindingsIsThreadSafeProviderAttribute : IBindingsAttribute
    {
        bool IsThreadSafe { get; set; }
    }

    // 是否自由函数提供者
    interface IBindingsIsFreeFunctionProviderAttribute : IBindingsAttribute
    {
        bool IsFreeFunction { get; set; }
        bool HasExplicitThis { get; set; } // 是否已实现
    }

    // 抛出提供者
    interface IBindingsThrowsProviderAttribute : IBindingsAttribute
    {
        bool ThrowsException { get; set; }
    }

    // 本地条件
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct | AttributeTargets.Method | AttributeTargets.Property)]
    [VisibleToOtherModules]
    class NativeConditionalAttribute : Attribute, IBindingsAttribute
    {
        public string Condition { get; set; }
        public string StubReturnStatement { get; set; }
        public bool Enabled { get; set; }

        public NativeConditionalAttribute()
        {
        }

        public NativeConditionalAttribute(string condition)
        {
            Condition = condition;
            Enabled = true;
        }

        public NativeConditionalAttribute(bool enabled)
        {
            Enabled = enabled;
        }

        public NativeConditionalAttribute(string condition, bool enabled) : this(condition)
        {
            Enabled = enabled;
        }

        public NativeConditionalAttribute(string condition, string stubReturnStatement, bool enabled) : this(condition, stubReturnStatement)
        {
            Enabled = enabled;
        }

        public NativeConditionalAttribute(string condition, string stubReturnStatement) : this(condition)
        {
            StubReturnStatement = stubReturnStatement;
        }
    }

    // 本地头文件
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct | AttributeTargets.Enum | AttributeTargets.Method | AttributeTargets.Property | AttributeTargets.Field | AttributeTargets.ReturnValue | AttributeTargets.Parameter, AllowMultiple = true)]
    [VisibleToOtherModules]
    class NativeHeaderAttribute : Attribute, IBindingsHeaderProviderAttribute
    {
        public string Header { get; set; }

        public NativeHeaderAttribute()
        {
        }

        public NativeHeaderAttribute(string header)
        {
            if (header == null) throw new ArgumentNullException("header");
            if (header == "") throw new ArgumentException("header cannot be empty", "header");

            Header = header;
        }
    }

    [VisibleToOtherModules]
    enum TargetType
    {
        Function,
        Field
    }

    // 本地方法
    [AttributeUsage(AttributeTargets.Method | AttributeTargets.Property)]
    [VisibleToOtherModules]
    class NativeMethodAttribute : Attribute, IBindingsNameProviderAttribute, IBindingsIsThreadSafeProviderAttribute, IBindingsIsFreeFunctionProviderAttribute, IBindingsThrowsProviderAttribute
    {
        public string Name { get; set; }
        public bool IsThreadSafe { get; set; }
        public bool IsFreeFunction { get; set; }
        public bool HasExplicitThis { get; set; }
        public bool ThrowsException { get; set; }

        public NativeMethodAttribute()
        {
        }

        public NativeMethodAttribute(string name)
        {
            if (name == null) throw new ArgumentNullException("name");
            if (name == "") throw new ArgumentException("name cannot be empty", "name");

            Name = name;
        }

        public NativeMethodAttribute(string name, bool isFreeFunction) : this(name)
        {
            IsFreeFunction = isFreeFunction;
        }

        public NativeMethodAttribute(string name, bool isFreeFunction, bool isThreadSafe) : this(name, isFreeFunction)
        {
            IsThreadSafe = isThreadSafe;
        }

        public NativeMethodAttribute(string name, bool isFreeFunction, bool isThreadSafe, bool throws) : this(name, isFreeFunction, isThreadSafe)
        {
            ThrowsException = throws;
        }
    }

    // 本地属性(属性的目标类型包含方法method和字段field)
    [AttributeUsage(AttributeTargets.Property)]
    [VisibleToOtherModules]
    class NativePropertyAttribute : NativeMethodAttribute
    {
        public TargetType TargetType { get; set; }

        public NativePropertyAttribute()
        {
        }

        public NativePropertyAttribute(string name) : base(name)
        {
        }

        public NativePropertyAttribute(string name, TargetType targetType) : base(name)
        {
            TargetType = targetType;
        }

        public NativePropertyAttribute(string name, bool isFree, TargetType targetType) : base(name, isFree)
        {
            TargetType = targetType;
        }

        public NativePropertyAttribute(string name, bool isFree, TargetType targetType, bool isThreadSafe) : base(name, isFree, isThreadSafe)
        {
            TargetType = targetType;
        }
    }

}
