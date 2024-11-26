using System;

using Volcano;

namespace Sandbox
{
    internal class Camera3D : MonoBehaviour
    {
        public float m_MouseSentity = 1.0f;
        public float m_MoveSpeed = 2.0f;
        private CameraComponent m_Camera;
        private Vector2 limitRotation;  //限制摄像机旋转的角度
        private GameObject m_Player;

        void Start()
        {
            m_Player = GameObject.Find("Player");
            if (m_Player != null)
                transform.LookAt(m_Player.transform);
            limitRotation = new Vector2(MathF.Radians(-89.0f), MathF.Radians(89.0f));
            m_Camera = GetComponent<CameraComponent>();
        }

        void Update(float ts)
        {
            // 视角rotation移动
            Mouse mouse = Mouse.Instance;
            bool leftAlt = Input.IsKeyPressed(KeyCode.LeftAlt);
            mouse.OnActive = leftAlt;
            if (!mouse.OnActive)
            {
                if (mouse.FirstMouse)
                {
                    mouse.LastX = mouse.MousePosition.x;
                    mouse.LastY = mouse.MousePosition.y;
                    mouse.FirstMouse = false;
                }

                float xoffset = mouse.LastX - mouse.MousePosition.x;
                float yoffset = mouse.LastY - mouse.MousePosition.y;
                mouse.LastX = mouse.MousePosition.x;
                mouse.LastY = mouse.MousePosition.y;

                xoffset *= m_MouseSentity;
                yoffset *= m_MouseSentity;

                // transform.rotation为弧度radians
                transform.RotateEulerAngle(MathF.Radians(yoffset), MathF.Radians(xoffset), 0);
                //rotation.x = MathF.Clamp(rotation.x, limitRotation.x, limitRotation.y);
            }
            else
            {
                mouse.FirstMouse = true;
            }


            // 坐标translation移动
            Vector3 velocity = Vector3.zero;

            if (Input.IsKeyPressed(KeyCode.W))
                velocity.z -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.S))
                velocity.z += 1.0f;

            if (Input.IsKeyPressed(KeyCode.A))
                velocity.x -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.D))
                velocity.x += 1.0f;

            if (Input.IsKeyPressed(KeyCode.LeftControl))
                velocity.y -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.Space))
                velocity.y += 1.0f;

            // velocity只在rotation(0,0,0)方向上移动,需要通过quatRotation转换方向

            // 四元数和向量相乘是向量按四元数进行了旋转
            // Quateration q = Quaternion.Euler(0, 90, 0) 绕着Y轴旋转90度
            Quaternion quatRotation = new Quaternion(new Vector3(0, transform.eulerAngles.y, 0));
            velocity = quatRotation * velocity;
            transform.Translate(velocity * m_MoveSpeed * ts, Space.World);


            //transform.parent.Translate(0.0f, 0.0f, m_MoveSpeed * ts);
            //transform.parent.RotateEulerAngle(m_MouseSentity * ts, 0.0f, 0.0f);
        }

        Vector3 GetUpDirection()
	    {
            return MathF.Rotate(GetOrientation(), Vector3.up);
	    }

	    Vector3 GetRightDirection()
	    {
	    	return MathF.Rotate(GetOrientation(), Vector3.right);
	    }

	    Vector3 GetForwardDirection()
	    {
	    	return MathF.Rotate(GetOrientation(), Vector3.forward);
	    }

	    Quaternion GetOrientation()
	    {
	    	return new Quaternion(new Vector3(MathF.Radians(transform.rotation.x), MathF.Radians(transform.rotation.y), MathF.Radians(transform.rotation.z)));
	    }

    }
}
