using System;
using Volcano;

namespace Sandbox
{
    internal class Gravity : MonoBehaviour
    {
        public float m_moveSpeed;

        void Update(float ts)
        {
            m_moveSpeed += 0.98f * ts;
            transform.Translate(new Vector3(0.0f, -1.0f, 0.0f) * m_moveSpeed * ts, Space.World);
        }

        void OnTriggerEnter(Collider other)
        {
            InternalCalls.DebugInfo("Gravity::OnTriggerEnter:" + other.ID.ToString());
        }
    }
}
