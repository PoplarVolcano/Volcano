using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Volcano;

namespace SandBox
{
    public class Player : Entity
    {
        private TransformComponent m_Transform;
        private Rigidbody2DComponent m_Rigidbody;

        // c#类被C++读取后会重新赋值并默认为0
        public float Speed;
        public float Time = 0.0f;

        void Start()
        {
            Console.WriteLine($"Player.OnCreate - {ID}");

            m_Transform = GetComponent<TransformComponent>();
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
        }

        void Update(float ts)
        {
            Time += ts;

            float speed = Speed;

            Vector3 velocity = Vector3.zero;

            if (Input.IsKeyPressed(KeyCode.W))
                velocity.y = 1.0f;
            else if (Input.IsKeyPressed(KeyCode.S))
                velocity.y = -1.0f;

            if (Input.IsKeyPressed(KeyCode.A))
                velocity.x = -1.0f;
            else if (Input.IsKeyPressed(KeyCode.D))
                velocity.x = 1.0f;

            Entity cameraEntity = FindEntityByName("Camera");
            
            if (cameraEntity != null)
            {
                // 如果没有camera实体调用camera脚本，则无法获得camera实体而报错
                Camera camera = cameraEntity.As<Camera>();

                if (Input.IsKeyPressed(KeyCode.Q))
                    camera.DistanceFromPlayer += speed * 2.0f * ts;
                else if (Input.IsKeyPressed(KeyCode.E))
                    camera.DistanceFromPlayer -= speed * 2.0f * ts;
            }

            velocity *= speed * ts;

            // 实体没有刚体组件时m_Rigidbody为空，直接调用方法会报错
            if (m_Rigidbody != null)
                m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);
            else
            {
                Vector3 localPosition = m_Transform.localPosition;
                localPosition += velocity * ts;
                m_Transform.localPosition = localPosition;
            }
        }

    }
}