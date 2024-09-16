using SandBox;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Volcano;

namespace Sandbox
{
    internal class FlashLight : Entity
    {
        private Entity m_Camera;
        void OnCreate()
        {
            m_Camera = FindEntityByName("Camera");
        }

        void OnUpdate(float ts)
        {
            if (m_Camera != null)
            {
                Vector3 translation = m_Camera.transform.translation;
                translation.y -= 1;
                transform.translation = translation;
                transform.rotation = m_Camera.transform.rotation;
            }

        }

    }
}
