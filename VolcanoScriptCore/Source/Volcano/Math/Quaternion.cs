using System;
using System.Globalization;
using System.Runtime.InteropServices;

namespace Volcano
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Quaternion : IEquatable<Quaternion>, IFormattable
    {
        public float x, y, z, w;

        private static readonly Quaternion identityQuaternion = new Quaternion(0f, 0f, 0f, 1f);

        // The identity Quaternion (Read Only).
        public static Quaternion identity { get => identityQuaternion; }

        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0: return x;
                    case 1: return y;
                    case 2: return z;
                    case 3: return w;
                    default: throw new IndexOutOfRangeException("Invalid Quaternion index!");
                }
                ;
            }
            set
            {
                switch (index)
                {
                    case 0: x = value; break;
                    case 1: y = value; break;
                    case 2: z = value; break;
                    case 3: w = value; break;
                    default: throw new IndexOutOfRangeException("Invalid Quaternion index!");
                }
            }
        }

        // Constructs new Quaternion with given x,y,z,w components.
        public Quaternion(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        //  Set x, y, z and w components of an existing Quaternion.
        public void Set(float newX, float newY, float newZ, float newW)
        {
            x = newX;
            y = newY;
            z = newZ;
            w = newW;
        }

        // ----------------------------- 乘法 ----------------------------- 

        // 四元数相乘，一个物体先绕某个轴旋转一定角度，再绕另一个轴（或同一轴的不同角度）旋转，最终得到的位置和方向由这两个旋转的复合决定。
        public static Quaternion operator *(Quaternion lhs, Quaternion rhs)
        {
            return new Quaternion(
                lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
                lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
                lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x,
                lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
                );
        }

        public static Vector3 operator *(Quaternion rotation, Vector3 point)
        {
            float num   = rotation.x * 2.0f;
            float num2  = rotation.y * 2.0f;
            float num3  = rotation.z * 2.0f;
            float num4  = rotation.x * num;
            float num5  = rotation.y * num2;
            float num6  = rotation.z * num3;
            float num7  = rotation.x * num2;
            float num8  = rotation.x * num3;
            float num9  = rotation.y * num3;
            float num10 = rotation.w * num;
            float num11 = rotation.w * num2;
            float num12 = rotation.w * num3;
            Vector3 result = default(Vector3);
            result.x = (1.0f - (num5 + num6)) * point.x + (num7 - num12) * point.y + (num8 + num11) * point.z;
            result.y = (num7 + num12) * point.x + (1.0f - (num4 + num6)) * point.y + (num9 - num10) * point.z;
            result.z = (num8 - num11) * point.x + (num9 + num10) * point.y + (1.0f - (num4 + num5)) * point.z;
            return result;
        }

        // ----------------------------- 点积 ----------------------------- 

        public static float Dot(Quaternion a, Quaternion b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        }

        // ----------------------------- 相等 ----------------------------- 

        private static bool IsEqualUsingDot(float dot)
        {
            return Math.Abs(dot) > 0.999999f;
        }

        public static bool operator ==(Quaternion lhs, Quaternion rhs)
        {
            return IsEqualUsingDot(Dot(lhs, rhs));
        }

        public static bool operator !=(Quaternion lhs, Quaternion rhs)
        {
            return !(lhs == rhs);
        }

        // 注：q  = (x,  y,  z,  w) 和 -q = (-x, -y, -z, -w) 表示的是完全相同的旋转。
        // 数学上，q 和 -q 作用在向量上结果相同：q * v * q⁻¹  ==  (-q) * v * (-q)⁻¹
        // Dot 的结果是两个单位四元数在 4D 单位球面上的夹角余弦。
        // Dot ==  1 → 完全相同
        // Dot == -1 → 表示同一个旋转（因为 q 和 - q）
        // Dot ==  0 → 完全无关
        // Unity 约定只要 |Dot| 接近 1 就算相等。
        public bool Equals(Quaternion other)
        {
            return IsEqualUsingDot(Dot(this, other));
        }

        public override bool Equals(object obj)
        {
            if (obj is Quaternion quat)
                return Equals(quat);
            return false;
        }

        // 只要重写了 Equals，编译器（和代码规范）就要求同时重写 GetHashCode
        // 17 和 31 是哈希魔术数，效果同 x.GetHashCode() ^ (y.GetHashCode() << 2) ^ (z.GetHashCode() >> 2) ^ (w.GetHashCode() >> 1);
        // unchecked 防止乘法溢出时报 OverflowException
        public override int GetHashCode()
        {
            unchecked   // 允许整数溢出（哈希计算里很常见）
            {
                int hash = 17;
                hash = hash * 31 + x.GetHashCode();
                hash = hash * 31 + y.GetHashCode();
                hash = hash * 31 + z.GetHashCode();
                hash = hash * 31 + w.GetHashCode();
                return hash;
            }
        }

        // ----------------------------- 归一化 / 逆 -----------------------------

        // .NET Framework 4.8 下，MathF 不存在于标准库中
        // 需安装微软官方提供的 Microsoft.Bcl.Numerics NuGet 包
        // Visual Studio 中，右键点击项目，选择 “管理 NuGet 程序包”
        // “浏览”选项卡中，搜索 Microsoft.Bcl.Numerics，然后安装
        public float Length => MathF.Sqrt(x * x + y * y + z * z + w * w);

        public Quaternion Normalized()
        {
            float length = Length;
            if (length < 1e-8f) 
                return identityQuaternion;
            float invLength = 1.0f / length;
            return new Quaternion(x * invLength, y * invLength, z * invLength, w * invLength);
        }

        public void Normalize()
        {
            float length = Length;
            if (length < 1e-8f)
            {
                this = identityQuaternion;
                return;
            }
            float invLength = 1.0f / length;
            Set(x * invLength, y * invLength, z * invLength, w * invLength);
        }

        // ----------------------------- 欧拉角 -----------------------------

        // 参数为弧度欧拉角，顺序 Z → X → Y（Unity 约定）
        public static Quaternion EulerRadians(Vector3 euler)
        {
            float halfX = euler.x * 0.5f;
            float halfY = euler.y * 0.5f;
            float halfZ = euler.z * 0.5f;

            float cx = MathF.Cos(halfX), sx = MathF.Sin(halfX);
            float cy = MathF.Cos(halfY), sy = MathF.Sin(halfY);
            float cz = MathF.Cos(halfZ), sz = MathF.Sin(halfZ);

            // Unity 组合顺序：qy * qx * qz
            return new Quaternion(
                cy * sx * cz + sy * cx * sz,
                sy * cx * cz - cy * sx * sz,
                cy * cx * sz - sy * sx * cz,
                cy * cx * cz + sy * sx * sz
            );
        }

        // 返回弧度欧拉角
        public static Vector3 ToEulerRadians(Quaternion quat)
        {
            // 归一化：四元数经过多次相乘后会积累浮点误差，长度不再是 1。
            quat.Normalize();

            // 化简后的矩阵运算，等价于从旋转矩阵里取"上下方向的投影"，即 sin(pitch)。
            //
            // 对于 Unity 约定的 ZXY 组合 quat = quat(y) * quat(x) * quat(z)，展开后的旋转矩阵是：
            //
            // Rx(pitch) = ⎡ 1    0        0     ⎤
            //             ⎢ 0   cos(x)  -sin(x) ⎥
            //             ⎣ 0   sin(x)   cos(x) ⎦
            // 
            // Ry(yaw)   = ⎡ cos(y)  0  sin(y) ⎤
            //             ⎢   0     1    0    ⎥
            //             ⎣ -sin(y) 0  cos(y) ⎦
            // 
            // Rz(roll)  = ⎡ cos(z)  -sin(z)  0 ⎤
            //             ⎢ sin(z)   cos(z)  0 ⎥
            //             ⎣   0        0     1 ⎦
            // 
            //      ⎡  cy·cz + sy·sx·sz   -cy·sz + sy·sx·cz    sy·cx ⎤
            // R =  ⎢  cx·sz                 cx·cz                -sx     ⎥
            //      ⎣ -sy·cz + cy·sx·sz    sy·sz + cy·sx·cz    cy·cx ⎦
            //
            // 其中
            // cx = cos(pitch)，sx = sin(pitch)
            // 
            // cy = cos(yaw)，  sy = sin(yaw)
            // 
            // cz = cos(roll)， sz = sin(roll)
            // 
            // 四元数转矩阵的通用公式：
            //
            //       ⎡ 1 - 2(y² +z²)   2(xy - wz)      2(xz + wy)    ⎤
            // R =   ⎢ 2(xy + wz)      1 - 2(x² +z²)   2(yz - wx)    ⎥
            //       ⎣ 2(xz - wy)      2(yz + wx)      1 - 2(x² +y²) ⎦
            //
            // R[1][2] = 2(y*z - w*x)
            //
            // => -sin(pitch) = 2(y*z - w*x)
            // 
            // R[0][2] = 2(x*z + w*y)  => sin(yaw)·cos(patch) = 2(x*z + w*y)
            // R[2][2] = 1 - 2(x² +y²) => cos(yaw)·cos(patch) = 1 - 2(x² +y²)
            //
            // =>tan(yaw) = 2(x*z + w*y) / (1 - 2(x² +y²))
            //
            // R[1][0] = 2(x * y + w * z) => cos(patch)·sin(roll) = 2(x * y + w * z)
            // R[1][1] = 1 - 2(x² +z²)    => cos(patch)·cos(roll) = 1 - 2(x² +z²)
            //
            // =>tan(roll) = 2(x * y + w * z) / (1 - 2(x² +y²))
            //

            float sinX = -2.0f  * (quat.y * quat.z - quat.w * quat.x);
            // 数值兜底：sin 的值域是 [-1, 1]，浮点误差可能让它变成 1.0000001
            sinX = MathFloat.Clamp(sinX, -1.0f , 1.0f );

            // 判断万向锁：当 |sinX| 接近 1，意味着俯仰角接近 ±90°，此时 yaw 轴和 roll 轴会重合，
            //             你无法再区分"这次旋转是偏航还是翻滚"
            Vector3 eulerRadians;
            if (MathF.Abs(sinX) > 0.99999f)
            {
                // 万向锁分支
                // 此时 yaw 和 roll 无法分开，干脆把 roll 归零，剩下的 yaw 由另一组公式算出，让朝向保持正确
                eulerRadians.x = MathFloat.CopySign(MathF.PI * 0.5f, sinX);
                eulerRadians.y = 2.0f  * MathF.Atan2(quat.y, quat.w);
                eulerRadians.z = 0.0f ;
            }
            else
            {
                // 【一般分支】
                // 三个角度各自独立算出
                // atan2(y, x) 返回的是 (x, y) 对应的极角，范围 (-π, π]
                // atan2 的参数是 旋转矩阵特定元素 的 两倍化简形式
                                eulerRadians.x = MathF.Asin(sinX);
                eulerRadians.y = MathF.Atan2(2.0f  * (quat.w * quat.y + quat.z * quat.x),
                                             1.0f  - 2.0f  * (quat.x * quat.x + quat.y * quat.y));
                eulerRadians.z = MathF.Atan2(2.0f  * (quat.w * quat.z + quat.x * quat.y),
                                             1.0f  - 2.0f  * (quat.x * quat.x + quat.z * quat.z));
            }
            return eulerRadians;
        }

        // ----------------------------- 轴角 -----------------------------

        // 参数为 角度值角 和 轴
        public static Quaternion AngleAxis(float angleDegree, Vector3 axis)
        {
            axis.Normalize();
            if (axis.Equals(Vector3.Zero))
                return identityQuaternion;

            float half = angleDegree * 0.5f * MathFloat.DegreeToRadian;
            float sinHalf = MathF.Sin(half);
            return new Quaternion(axis.x * sinHalf, axis.y * sinHalf, axis.z * sinHalf, MathF.Cos(half));
        }

        // 将欧拉角调整为正0/360，并添加0.0001以支持QuaternionToEuler的旧行为
        private static Vector3 MakePositive(Vector3 euler)
        {
            float negativeFlip = -0.0001f * MathFloat.RadianToDegree;
            float positiveFlip = 360.0f + negativeFlip;

            if (euler.x < negativeFlip)
                euler.x += 360.0f;
            else if (euler.x > positiveFlip)
                euler.x -= 360.0f;

            if (euler.y < negativeFlip)
                euler.y += 360.0f;
            else if (euler.y > positiveFlip)
                euler.y -= 360.0f;

            if (euler.z < negativeFlip)
                euler.z += 360.0f;
            else if (euler.z > positiveFlip)
                euler.z -= 360.0f;

            return euler;
        }

        public Vector3 eulerAngles
        {
            get => MakePositive(ToEulerRadians(this) * MathFloat.RadianToDegree);
            set => this = EulerRadians(value * MathFloat.DegreeToRadian);
        }

        public static Quaternion Euler(float x, float y, float z) { 
            return EulerRadians(new Vector3(x, y, z) * MathFloat.DegreeToRadian);
        }

        public static Quaternion Euler(Vector3 euler) {
            return EulerRadians(euler * MathFloat.DegreeToRadian);
        }

        // ----------------------------- 格式化 -----------------------------

        public override string ToString()
        {
            return ToString(null, null);
        }

        public string ToString(string format, IFormatProvider formatProvider)
        {
            if (string.IsNullOrEmpty(format))
            {
                format = "F5";
            }

            if (formatProvider == null)
            {
                formatProvider = CultureInfo.InvariantCulture.NumberFormat;
            }

            return string.Format("({0}, {1}, {2}, {3})",
                x.ToString(format, formatProvider),
                y.ToString(format, formatProvider),
                z.ToString(format, formatProvider),
                w.ToString(format, formatProvider));
        }

    }
}
