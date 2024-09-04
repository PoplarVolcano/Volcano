using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using Volcano;

namespace SandBox
{
    internal class Camera3D : Entity
    {
        public float m_MouseSentity = 0.1f;
        public float m_MoveSpeed = 2.0f;
        private CameraComponent m_Camera;
        private Vector2 limitRotation;  //�����������ת�ĽǶ�

        void OnCreate()
        {
            limitRotation = new Vector2(MathF.Radians(-89.0f), MathF.Radians(89.0f));
            m_Camera = GetComponent<CameraComponent>();
        }

        void OnUpdate(float ts)
        {
            // �ӽ�rotation�ƶ�
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

                // transform.rotationΪ����radians
                Vector3 rotation = transform.rotation;
                rotation.x += MathF.Radians(yoffset);
                rotation.y += MathF.Radians(xoffset);
                //rotation.x = MathF.Clamp(rotation.x, limitRotation.x, limitRotation.y);
                transform.rotation = rotation;
            }
            else
            {
                mouse.FirstMouse = true;
            }


            // ����translation�ƶ�
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



            // velocityֻ��rotation(0,0,0)�������ƶ�,��Ҫͨ��quatRotationת������

            // ��Ԫ���������������������Ԫ����������ת
            // Quateration q = Quaternion.Euler(0, 90, 0) ����Y����ת90��
            Quaternion quatRotation = new Quaternion(new Vector3(0, transform.rotation.y, 0));
            velocity = quatRotation * velocity;
            Vector3 translation = transform.translation;
            translation += velocity * m_MoveSpeed * ts;
            transform.translation = translation;

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
