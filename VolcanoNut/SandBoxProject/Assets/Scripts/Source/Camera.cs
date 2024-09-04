using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Volcano;

namespace SandBox
{
    internal class Camera : Entity
    {
        public float DistanceFromPlayer = 5.0f;
        private Entity m_Player;

        void OnCreate()
        {
            m_Player = FindEntityByName("Player");
        }

        void OnUpdate(float ts)
        {
            Mouse.Instance.OnActive = Input.IsKeyPressed(KeyCode.LeftAlt) || Input.IsKeyPressed(KeyCode.RightAlt);

            TransformComponent tc = GetComponent<TransformComponent>();
            if (m_Player != null)
                transform.translation = new Vector3(m_Player.transform.translation.XY, DistanceFromPlayer);

            float speed = 1.0f;
            Vector3 velocity = Vector3.zero;

            if (Input.IsKeyPressed(KeyCode.Up))
                velocity.z -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.Down))
                velocity.z += 1.0f;

            if (Input.IsKeyPressed(KeyCode.Left))
                velocity.x -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.Right))
                velocity.x += 1.0f;

            if (Input.IsKeyPressed(KeyCode.LeftControl))
                velocity.y -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.Space))
                velocity.y += 1.0f;


            Vector3 translation = transform.translation;
            translation += velocity * speed * ts;
            transform.translation = translation;
        }
    }
}
