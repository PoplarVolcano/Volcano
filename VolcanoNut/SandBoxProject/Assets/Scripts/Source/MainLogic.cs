using System;
using Volcano;

namespace Sandbox
{
    public class MainLogic : Entity
    {
        public float test = 10.0f;
        public Entity target;
        public Entity bulletPrefab;


        private Entity entity;

        private void Awake()
        {
            test = 20.0f;
            //target = FindEntityByName("Player");
            entity = FindEntityByName("游戏主控");
            if(entity != null)
                entity.transform.position += Vector3.up;
            Application.targetFrameRate = 60;

            InternalCalls.DebugTrace("MainLogic.Awake:" + ID);
        }

        void Start()
        {
            test = 40.0f;

            InternalCalls.DebugTrace("MainLogic.Start:" + ID);
        }

        void Update(float ts)
        {
            if(Input.IsMouseButtonPressed(0))
            {
                Entity node = Instantiate(bulletPrefab, null);
                node.transform.position = Vector3.zero;
                node.transform.localEulerAngles = Vector3.zero;
            }

            /*
            if (target != null)
                target.transform.position += Vector3.up;

            Entity[] children = this.children;
            if (children != null)
            {
                children[0].transform.position += Vector3.up;
            }
            else
                InternalCalls.DebugTrace("children is null");
            */
        }

        void OnEnable()
        {
            test = 30.0f;
        }

    }
}
