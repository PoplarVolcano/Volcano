using System;
using Volcano;

namespace Sandbox
{
    internal class BulletLogic : Entity
    {
        public float speed;

        void Start()
        {

        }

        void Update(float ts)
        {
            this.transform.Translate(Vector3.forward * speed * ts);
        }
    }
}
