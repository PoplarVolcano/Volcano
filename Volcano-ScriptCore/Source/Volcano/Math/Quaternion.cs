using System;
using System.Globalization;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Volcano
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Quaternion : IEquatable<Quaternion>, IFormattable
    {
        public float x;
        public float y;
        public float z;
        public float w;

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
                };
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

        // 四元数相乘，一个物体先绕某个轴旋转一定角度，再绕另一个轴（或同一轴的不同角度）旋转，最终得到的位置和方向由这两个旋转的复合决定。
        public static Quaternion operator *(Quaternion lhs, Quaternion rhs)
        {
            return new Quaternion(lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y, lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z, lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x, lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z);
        }

        public static Vector3 operator *(Quaternion rotation, Vector3 point)
        {
            float num   = rotation.x * 2f;
            float num2  = rotation.y * 2f;
            float num3  = rotation.z * 2f;
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
            result.x = (1f - (num5 + num6)) * point.x + (num7 - num12) * point.y + (num8 + num11) * point.z;
            result.y = (num7 + num12) * point.x + (1f - (num4 + num6)) * point.y + (num9 - num10) * point.z;
            result.z = (num8 - num11) * point.x + (num9 + num10) * point.y + (1f - (num4 + num5)) * point.z;
            return result;
        }

        public static float Dot(Quaternion a, Quaternion b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        }

        private static bool IsEqualUsingDot(float dot)
        {
            return dot > 0.999999f;
        }

        public static bool operator ==(Quaternion lhs, Quaternion rhs)
        {
            return IsEqualUsingDot(Dot(lhs, rhs));
        }

        public static bool operator !=(Quaternion lhs, Quaternion rhs)
        {
            return !(lhs == rhs);
        }


        // Makes euler angles positive 0/360 with 0.0001 hacked to support old behaviour of QuaternionToEuler
        private static Vector3 Internal_MakePositive(Vector3 euler)
        {
            float negativeFlip = -0.0001f * MathF.Rad2Deg;
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
            get { return Internal_MakePositive(MathF.EulerFromQuaternion(this) * MathF.Rad2Deg); }
            set { this = MathF.QuaternionFromEuler(value * MathF.Deg2Rad); }
        }

        public static Quaternion Euler(float x, float y, float z) { return MathF.QuaternionFromEuler(new Vector3(x, y, z) * MathF.Deg2Rad); }
        public static Quaternion Euler(Vector3 euler) { return MathF.QuaternionFromEuler(euler * MathF.Deg2Rad); }





        public bool Equals(Quaternion other)
        {
            return x.Equals(other.x) && y.Equals(other.y) && z.Equals(other.z) && w.Equals(other.w);
        }

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

            return string.Format("({0}, {1}, {2}, {3})", x.ToString(format, formatProvider), y.ToString(format, formatProvider), z.ToString(format, formatProvider), w.ToString(format, formatProvider));
        }

        public Quaternion(Vector3 Euler)
        {
            this = MathF.QuaternionFromEuler(Euler);
        }
    }
}
