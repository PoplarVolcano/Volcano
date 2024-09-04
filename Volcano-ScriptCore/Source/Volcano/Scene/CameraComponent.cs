using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public class CameraComponent : Component
    {
        enum ProjectionType { Prespective = 0, Orthographic = 1 };
        ProjectionType m_ProjectionType { get; set; }

        // field of view
        public float m_PerspectiveFOV { get; set; }
        public float m_PerspectiveNear { get; set; }
        public float m_PerspectiveFar { get; set; }
        public float m_OrthographicSize { get; set; }
        public float m_OrthographicNear { get; set; }
        public float m_OrthographicFar { get; set; }
        public float m_AspectRatio { get; set; }

        public Matrix4x4 m_View { get;  set; }


    }
}
