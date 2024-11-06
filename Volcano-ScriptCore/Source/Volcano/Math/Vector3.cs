using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Volcano.Bindings;
using Volcano.Scripting;

namespace Volcano
{
    // Representation of 3D vectors and points.
    [NativeHeader("Runtime/Math/Vector3.h")]
    [NativeClass("Vector3f")]
    [RequiredByNativeCode(Optional = true, GenerateProxy = true)]
    [StructLayout(LayoutKind.Sequential)]
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

        public static Vector3 zero    { get => zeroVector;    }
        public static Vector3 one     { get => oneVector;     }
        public static Vector3 up      { get => upVector;      }
        public static Vector3 down    { get => downVector;    }
        public static Vector3 left    { get => leftVector;    }
        public static Vector3 right   { get => rightVector;   }
        public static Vector3 forward { get => forwardVector; }
        public static Vector3 back    { get => backVector;    }

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

        // Returns a copy of /vector/ with its magnitude clamped to /maxLength/.
        public static Vector3 ClampMagnitude(Vector3 vector, float maxLength)
        {
            float sqrmag = vector.sqrMagnitude;
            if (sqrmag > maxLength * maxLength)
            {
                float mag = (float)Math.Sqrt(sqrmag);
                //these intermediate variables force the intermediate result to be
                //of float precision. without this, the intermediate result can be of higher
                //precision, which changes behavior.
                float normalized_x = vector.x / mag;
                float normalized_y = vector.y / mag;
                float normalized_z = vector.z / mag;
                return new Vector3(normalized_x * maxLength,
                    normalized_y * maxLength,
                    normalized_z * maxLength);
            }
            return vector;
        }

        // Returns the length of this vector.
        public float magnitude { get { return (float)Math.Sqrt(x * x + y * y + z * z); } }

        // Returns the squared length of this vector.
        public float sqrMagnitude { get { return x * x + y * y + z * z; } }


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
                };
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
            throw new NotImplementedException();
        }

        public string ToString(string format, IFormatProvider formatProvider)
        {
            throw new NotImplementedException();
        }
    }
}