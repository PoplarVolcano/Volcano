using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Volcano;

namespace Sandbox
{
    internal class FlashLight : MonoBehaviour
    {
        private GameObject m_Camera;
        void OnCreate()
        {
            m_Camera = GameObject.Find("Camera");
        }

        void OnUpdate(float ts)
        {
            if (m_Camera != null)
            {
                Vector3 localPosition = m_Camera.transform.localPosition;
                //localPosition.y -= 1;
                transform.localPosition = localPosition;
                transform.rotation = m_Camera.transform.rotation;
            }

        }

    }
}
