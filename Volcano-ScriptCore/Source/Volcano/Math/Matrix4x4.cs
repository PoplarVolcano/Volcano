using System;
using System.Globalization;

namespace Volcano
{
    public struct Matrix4x4 : IEquatable<Matrix4x4>, IFormattable
    {
        public float m00;
        public float m10;
        public float m20;
        public float m30;

        public float m01;
        public float m11;
        public float m21;
        public float m31;

        public float m02;
        public float m12;
        public float m22;
        public float m32;

        public float m03;
        public float m13;
        public float m23;
        public float m33;

        private static readonly Matrix4x4 zeroMatrix     = new Matrix4x4(new Vector4(0f, 0f, 0f, 0f), new Vector4(0f, 0f, 0f, 0f), new Vector4(0f, 0f, 0f, 0f), new Vector4(0f, 0f, 0f, 0f));
        private static readonly Matrix4x4 identityMatrix = new Matrix4x4(new Vector4(1f, 0f, 0f, 0f), new Vector4(0f, 1f, 0f, 0f), new Vector4(0f, 0f, 1f, 0f), new Vector4(0f, 0f, 0f, 1f));
        public static Matrix4x4 zero     { get => zeroMatrix;     }
        public static Matrix4x4 identity { get => identityMatrix; }

        public float this[int index]
        {
            get
            {
                switch (index)
                {
                    case 0 : return m00;
                    case 1 : return m10;
                    case 2 : return m20;
                    case 3 : return m30;
                    case 4 : return m01;
                    case 5 : return m11;
                    case 6 : return m21;
                    case 7 : return m31;
                    case 8 : return m02;
                    case 9 : return m12;
                    case 10: return m22;
                    case 11: return m32;
                    case 12: return m03;
                    case 13: return m13;
                    case 14: return m23;
                    case 15: return m33;
                    default: throw new IndexOutOfRangeException("Invalid matrix index!");
                };
            }
            set
            {
                switch (index)
                {
                    case 0:  m00 = value; break;
                    case 1:  m10 = value; break;
                    case 2:  m20 = value; break;
                    case 3:  m30 = value; break;
                    case 4:  m01 = value; break;
                    case 5:  m11 = value; break;
                    case 6:  m21 = value; break;
                    case 7:  m31 = value; break;
                    case 8:  m02 = value; break;
                    case 9:  m12 = value; break;
                    case 10: m22 = value; break;
                    case 11: m32 = value; break;
                    case 12: m03 = value; break;
                    case 13: m13 = value; break;
                    case 14: m23 = value; break;
                    case 15: m33 = value; break;
                    default: throw new IndexOutOfRangeException("Invalid matrix index!");
                }
            }
        }

        public Matrix4x4(Vector4 column0, Vector4 column1, Vector4 column2, Vector4 column3)
        {
            m00 = column0.x;
            m01 = column1.x;
            m02 = column2.x;
            m03 = column3.x;
            m10 = column0.y;
            m11 = column1.y;
            m12 = column2.y;
            m13 = column3.y;
            m20 = column0.z;
            m21 = column1.z;
            m22 = column2.z;
            m23 = column3.z;
            m30 = column0.w;
            m31 = column1.w;
            m32 = column2.w;
            m33 = column3.w;
        }

        // 取逆
        public static Matrix4x4 Inverse(Matrix4x4 m)
        {
            return MathF.Inverse(m);
        }

        // 转置
        public static Matrix4x4 Transpose(Matrix4x4 m)
        {
            return MathF.Transpose(m);
        }

        // 透视投影矩阵
        public static Matrix4x4 Perspective(float fov, float aspect, float zNear, float zFar)
        {
            return MathF.Perspective(fov, aspect, zNear, zFar);
        }

        public Matrix4x4 inverse => Inverse(this);
        public Matrix4x4 transpose => Transpose(this);

        public Vector4 GetColumn(int index)
        {
            switch (index)
            {
                case 0: return new Vector4(m00, m10, m20, m30);
                case 1: return new Vector4(m01, m11, m21, m31);
                case 2: return new Vector4(m02, m12, m22, m32);
                case 3: return new Vector4(m03, m13, m23, m33);
                default: throw new IndexOutOfRangeException("Invalid column index!");
            };
        }

        public Vector4 GetRow(int index)
        {
            switch (index)
            {
                case 0: return new Vector4(m00, m01, m02, m03);
                case 1: return new Vector4(m10, m11, m12, m13);
                case 2: return new Vector4(m20, m21, m22, m23);
                case 3: return new Vector4(m30, m31, m32, m33);
                default: throw new IndexOutOfRangeException("Invalid row index!");
            };
        }

        public bool Equals(Matrix4x4 other)
        {
            return GetColumn(0).Equals(other.GetColumn(0)) && GetColumn(1).Equals(other.GetColumn(1)) && GetColumn(2).Equals(other.GetColumn(2)) && GetColumn(3).Equals(other.GetColumn(3));
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

            return string.Format(
                "{0}\t{1}\t{2}\t{3}\n{4}\t{5}\t{6}\t{7}\n{8}\t{9}\t{10}\t{11}\n{12}\t{13}\t{14}\t{15}\n", 
                m00.ToString(format, formatProvider), m01.ToString(format, formatProvider), m02.ToString(format, formatProvider), m03.ToString(format, formatProvider), 
                m10.ToString(format, formatProvider), m11.ToString(format, formatProvider), m12.ToString(format, formatProvider), m13.ToString(format, formatProvider), 
                m20.ToString(format, formatProvider), m21.ToString(format, formatProvider), m22.ToString(format, formatProvider), m23.ToString(format, formatProvider), 
                m30.ToString(format, formatProvider), m31.ToString(format, formatProvider), m32.ToString(format, formatProvider), m33.ToString(format, formatProvider));
        }
    }
}
