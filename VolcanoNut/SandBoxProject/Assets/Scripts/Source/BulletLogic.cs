using System;
using Volcano;

namespace Sandbox
{
    internal class BulletLogic : MonoBehaviour
    {
        public float speed = 0f;
        public Transform bulletFolder;

        void OnEnable()
        {
            this.Invoke("ObjectOff", 1.0f);
            //this.transform.parent = null;
        }

        void Start()
        {
            //this.Invoke("Destroy", 3.0f);
        }

        void Update(float ts)
        {
            if (speed != 0f)
            {
                this.transform.Translate(this.transform.rotation * Vector3.forward * speed * ts, Space.Self);
            }
        }

        void OnDisable()
        {
            //this.transform.parent = bulletFolder;
            //this.transform.localPosition = Vector3.zero;
            //this.transform.localRotation = Quaternion.identity;
            //speed = 0f;
        }

        void ObjectOff()
        {
            this.transform.localPosition = Vector3.zero;
            //this.transform.parent = bulletFolder;
            this.gameObject.active = false;
        }
    }
}
