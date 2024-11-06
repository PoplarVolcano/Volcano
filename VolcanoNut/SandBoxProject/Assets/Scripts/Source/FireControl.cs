using System;
using Volcano;

namespace Sandbox
{
    internal class FireControl : Entity
    {
        public Entity bulletPrefab;
        public TransformComponent bulletFolder;
        public TransformComponent cannon;
        public TransformComponent firePoint;

        void Start()
        {


        }

        void Update(float ts)
        {

        }

        private void Fire()
        {
            InternalCalls.DebugInfo("* 创建子弹的实例 ..");
            Entity node = Entity.Instantiate(bulletPrefab, bulletFolder);  // 指定父节点，创建实例
            node.transform.position = this.firePoint.position;             // 指定出生点位置
            node.transform.eulerAngles = this.cannon.eulerAngles;          // 与炮塔角度一致
            //node.transform.rotation = this.cannon.rotation;

            //BulletLogic script = node.GetComponent<BulletLogic>();

        }
    }
}
