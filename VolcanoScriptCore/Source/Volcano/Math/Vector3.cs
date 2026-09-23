using System;

namespace Volcano
{
    public partial struct Vector3 : IEquatable<Vector3>, IFormattable
    {
        public float x, y, z;

        //Epsilon是希腊字母中的第五个字母，常用于表示极小值或接近于零的数。
        public const float kEpsilon = 0.00001F;
        public const float kEpsilonNormalSqrt = 1e-15F;

        private static readonly Vector3 zeroVector    = new Vector3( 0.0f,  0.0f,  0.0f);
        private static readonly Vector3 oneVector     = new Vector3( 1.0f,  1.0f,  1.0f);
        private static readonly Vector3 upVector      = new Vector3( 0.0f,  1.0f,  0.0f);
        private static readonly Vector3 downVector    = new Vector3( 0.0f, -1.0f,  0.0f);
        private static readonly Vector3 leftVector    = new Vector3(-1.0f,  0.0f,  0.0f);
        private static readonly Vector3 rightVector   = new Vector3( 1.0f,  0.0f,  0.0f);
        private static readonly Vector3 forwardVector = new Vector3( 0.0f,  0.0f, -1.0f);
        private static readonly Vector3 backVector    = new Vector3( 0.0f,  0.0f,  1.0f);

        public static Vector3 Zero    { get => zeroVector;    }
        public static Vector3 One     { get => oneVector;     }
        /// <summary>0.0f, 1.0f, 0.0f</summary>
        public static Vector3 Up      { get => upVector; }
        /// <summary>0.0f, -1.0f, 0.0f</summary>
        public static Vector3 Down    { get => downVector; }
        /// <summary>-1.0f, 0.0f, 0.0f</summary>
        public static Vector3 Left    { get => leftVector; }
        /// <summary> 1.0f, 0.0f, 0.0f</summary>
        public static Vector3 Right   { get => rightVector; }
        /// <summary>0.0f, 0.0f, -1.0f</summary>
        public static Vector3 Forward { get => forwardVector; }
        /// <summary>0.0f, 0.0f, 1.0f</summary>
        public static Vector3 Back    { get => backVector;    }

        public Vector3(float scalar)
        {
            x = scalar;
            y = scalar;
            z = scalar;
        }

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public Vector3(Vector2 xy, float z)
        {
            this.x = xy.x;
            this.y = xy.y;
            this.z = z;
        }

        public Vector2 XY
        {
            get => new Vector2(x, y);
            set
            {
                x = value.x;
                y = value.y;
            }
        }

        /// <summary>返回一个其大小被限制为/maxLength/的/vector/的副本。</summary>
        public static Vector3 ClampMagnitude(Vector3 vector, float maxLength)
        {
            float sqrmag = vector.SqrMagnitude;
            if (sqrmag > maxLength * maxLength)
            {
                float mag = (float)Math.Sqrt(sqrmag);
                // 这些中间变量迫使中间结果具有浮点精度。若没有这些变量，中间结果可能会具有更高的精度，从而改变行为。
                float normalized_x = vector.x / mag;
                float normalized_y = vector.y / mag;
                float normalized_z = vector.z / mag;
                return new Vector3(normalized_x * maxLength,
                    normalized_y * maxLength,
                    normalized_z * maxLength);
            }
            return vector;
        }

        /// <summary>向量长度</summary>
        public float Magnitude { get { return (float)Math.Sqrt(x * x + y * y + z * z); } }

        /// <summary>平方和</summary>
        public float SqrMagnitude { get { return x * x + y * y + z * z; } }

        public Vector3 Normalized
        {
            get { return MathFloat.NormalizedVector3(this); }
        }

        public void Normalize()
        {
            float magnitude = Magnitude;
            if (magnitude > 1e-6f)   // 防止除以 0（零向量、极短向量）
            {
                x /= magnitude;
                y /= magnitude;
                z /= magnitude;
            }
            else
            {
                x = 0f;
                y = 0f;
                z = 0f;
            }
        }

        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        public static Vector3 operator -(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        public static Vector3 operator -(Vector3 a)
        {
            return new Vector3(-a.x, -a.y, -a.z);
        }

        public static Vector3 operator *(Vector3 vector, float scalar)
        {
            return new Vector3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
        }

        public static Vector3 operator *(Vector3 vector1, Vector3 vector2)
        {
            return new Vector3(vector1.x * vector2.x, vector1.y * vector2.y, vector1.z * vector2.z);
        }

        public static bool operator ==(Vector3 vector1, Vector3 vector2)
        {
            return vector1.x == vector2.x && vector1.y == vector2.y && vector1.z == vector2.z;
        }

        public static bool operator !=(Vector3 vector1, Vector3 vector2)
        {
            return vector1.x != vector2.x || vector1.y != vector2.y || vector1.z != vector2.z;
        }

        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0: return x;
                    case 1: return y;
                    case 2: return z;
                    default: throw new IndexOutOfRangeException("Invalid Vector3 index!");
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
                    default: throw new IndexOutOfRangeException("Invalid Vector3 index!");
                }
            }
        }

        public bool Equals(Vector3 other)
        {
            return x == other.x && y == other.y && z == other.z;
        }

        public string ToString(string format, IFormatProvider formatProvider)
        {
            throw new NotImplementedException();
        }
    }
}