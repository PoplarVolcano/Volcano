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

        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {ID}");

            m_Transform = GetComponent<TransformComponent>();
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
        }

        void OnUpdate(float ts)
        {
            Time += ts;

            float speed = Speed;

            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.Y = 1.0f;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.Y = -1.0f;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X = -1.0f;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X = 1.0f;

            Entity cameraEntity = FindEntityByName("Camera");
            
            if (cameraEntity != null)
            {
                // 如果没有camera实体调用camera脚本，则无法获得camera实体而报错
                Camera camera = cameraEntity.As<Camera>();

                if (Input.IsKeyDown(KeyCode.Q))
                    camera.DistanceFromPlayer += speed * 2.0f * ts;
                else if (Input.IsKeyDown(KeyCode.E))
                    camera.DistanceFromPlayer -= speed * 2.0f * ts;
            }

            velocity *= speed * ts;

            // 实体没有刚体组件时m_Rigidbody为空，直接调用方法会报错
            if (m_Rigidbody != null)
                m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);
            else
            {
                Vector3 translation = m_Transform.Translation;
                translation += velocity * ts;
                m_Transform.Translation = translation;
            }
        }

    }
}