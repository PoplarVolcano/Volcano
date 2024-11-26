using System;

namespace Volcano
{
    //generic casts in our mono carry a uncomfortable performance penalty. in some cases, we are sure the cast will succeed. we use the
    //trick with the struct below to get a cast done without any checks in that case, which gets us a performance increase.
    // mono中通常的cast会带来令人不适的性能损失。在某些情况下，我们确信cast会成功。
    // 在这种情况下，我们通过使用下面结构体的技巧在不进行任何检查的情况下完成强制转换，从而提高了性能。
    internal struct CastHelper<T>
    {
        public T t;
        public System.IntPtr onePointerFurtherThanT;
    }
}
