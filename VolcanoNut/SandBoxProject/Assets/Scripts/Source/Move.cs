using System;

using Volcano;

namespace Sandbox
{
    internal class Move : MonoBehaviour
    {
        public float m_MoveSpeed = 2.0f;


        void Update(float ts)
        {
            // 坐标translation移动
            Vector3 velocity = Vector3.zero;

            if (Input.IsKeyPressed(KeyCode.I))
                velocity.z -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.K))
                velocity.z += 1.0f;

            if (Input.IsKeyPressed(KeyCode.J))
                velocity.x -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.L))
                velocity.x += 1.0f;

            if (Input.IsKeyPressed(KeyCode.U))
                velocity.y -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.O))
                velocity.y += 1.0f;

            // 四元数和向量相乘是向量按四元数进行了旋转
            // Quateration q = Quaternion.Euler(0, 90, 0) 绕着Y轴旋转90度
            //Quaternion quatRotation = new Quaternion(new Vector3(0, transform.eulerAngles.y, 0));
            //velocity = quatRotation * velocity;
            transform.Translate(velocity * m_MoveSpeed * ts, Space.World);

        }

    }
}
