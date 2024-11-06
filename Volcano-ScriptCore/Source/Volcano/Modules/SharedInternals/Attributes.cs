using System;
using Volcano.Bindings;

namespace Volcano.Scripting
{
    // 被本地代码使用
    [AttributeUsage(AttributeTargets.Method | AttributeTargets.Class | AttributeTargets.Field | AttributeTargets.Struct | AttributeTargets.Property | AttributeTargets.Constructor | AttributeTargets.Interface | AttributeTargets.Enum, Inherited = false)]
    [VisibleToOtherModules]
    internal class UsedByNativeCodeAttribute : Attribute
    {
        public UsedByNativeCodeAttribute()
        {
        }

        public UsedByNativeCodeAttribute(string name)
        {
            Name = name;
        }

        public string Name { get; set; }
    }

    // 被本地代码依赖
    [AttributeUsage(AttributeTargets.Method | AttributeTargets.Class | AttributeTargets.Field | AttributeTargets.Struct | AttributeTargets.Property | AttributeTargets.Constructor | AttributeTargets.Interface | AttributeTargets.Enum, Inherited = false)]
    [VisibleToOtherModules]
    internal class RequiredByNativeCodeAttribute : Attribute
    {
        public RequiredByNativeCodeAttribute()
        {
        }

        public RequiredByNativeCodeAttribute(string name)
        {
            Name = name;
        }

        public RequiredByNativeCodeAttribute(bool optional)
        {
            Optional = optional;
        }

        public RequiredByNativeCodeAttribute(string name, bool optional)
        {
            Name = name;
            Optional = optional;
        }

        public string Name { get; set; }
        public bool Optional { get; set; } // 可选择的
        public bool GenerateProxy { get; set; } // 生成代理
    }


    // 本地类
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct, Inherited = false)]
    [VisibleToOtherModules]
    internal sealed class NativeClassAttribute : Attribute
    {
        public string QualifiedNativeName { get; private set; }  // 合格的本地名称
        public string Declaration { get; private set; }  // 声明

        public NativeClassAttribute(string qualifiedCppName)
        {
            QualifiedNativeName = qualifiedCppName;
            Declaration = "class " + qualifiedCppName;
        }

        public NativeClassAttribute(string qualifiedCppName, string declaration)
        {
            QualifiedNativeName = qualifiedCppName;
            Declaration = declaration;
        }
    }
}
