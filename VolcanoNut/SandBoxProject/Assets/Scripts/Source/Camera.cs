﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Volcano;

namespace Sandbox
{
    internal class Camera : MonoBehaviour
    {
        public float DistanceFromPlayer = 5.0f;
        private GameObject m_Player;

        void Start()
        {
            m_Player = GameObject.Find("Player");
            if(m_Player != null)
                transform.LookAt(m_Player.transform);
        }

        void Update(float ts)
        {
            Mouse.Instance.OnActive = Input.IsKeyPressed(KeyCode.LeftAlt) || Input.IsKeyPressed(KeyCode.RightAlt);

            Transform tc = GetComponent<Transform>();
            if (m_Player != null)
                transform.localPosition = new Vector3(m_Player.transform.localPosition.XY, DistanceFromPlayer);

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


            Vector3 localPosition = transform.localPosition;
            localPosition += velocity * speed * ts;
            transform.localPosition = localPosition;
        }
    }
}
