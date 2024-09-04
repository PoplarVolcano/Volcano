using System;
using System.Globalization;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public struct Vector4 : IEquatable<Vector4>, IFormattable
    {
        public float x;
        public float y;
        public float z;
        public float w;

        private static readonly Vector4 zeroVector = new Vector4(0f, 0f, 0f, 0f);
        private static readonly Vector4 oneVector = new Vector4(1f, 1f, 1f, 1f);
        public static Vector4 zero { get { return zeroVector; } }
        public static Vector4 one  { get { return oneVector;  } }

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
                    default: throw new IndexOutOfRangeException("Invalid Vector4 index!");
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
                    default: throw new IndexOutOfRangeException("Invalid Vector4 index!");
                }
            }
        }

        public Vector4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public Vector4(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            w = 0f;
        }

        public Vector4(float x, float y)
        {
            this.x = x;
            this.y = y;
            z = 0f;
            w = 0f;
        }

        public void Set(float newX, float newY, float newZ, float newW)
        {
            x = newX;
            y = newY;
            z = newZ;
            w = newW;
        }

        public bool Equals(Vector4 other)
        {
            return x == other.x && y == other.y && z == other.z && w == other.w; ;
        }

        public string ToString(string format, IFormatProvider formatProvider)
        {
            if (string.IsNullOrEmpty(format))
            {
                format = "F2";
            }

            if (formatProvider == null)
            {
                formatProvider = CultureInfo.InvariantCulture.NumberFormat;
            }

            return string.Format("({0}, {1}, {2}, {3})", x.ToString(format, formatProvider), y.ToString(format, formatProvider), z.ToString(format, formatProvider), w.ToString(format, formatProvider));
        }
    }
}
