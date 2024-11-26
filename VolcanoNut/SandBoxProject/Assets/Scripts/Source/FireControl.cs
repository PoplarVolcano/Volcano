using System;
using Volcano;

namespace Sandbox
{
    internal class FireControl : MonoBehaviour
    {
        public GameObject bulletPrefab;
        public Transform bulletFolder;
        public Transform cannon;
        public Transform firePoint;

        void Start()
        {


        }

        void Update(float ts)
        {

        }

        public void Fire()
        {
            if (this.bulletFolder == null || this.bulletFolder.childCount == 0 || this.cannon == null || this.firePoint == null)
                return;

            for (int i = bulletFolder.childCount - 1; i >= 0; i--)
            {
                GameObject bullet = bulletFolder.GetChild(i).gameObject;
                if (!bullet.active)
                {
                    bullet.active = true;
                    bullet.transform.position = this.firePoint.position;             // 指定出生点位置
                    bullet.transform.eulerAngles = this.cannon.eulerAngles;          // 与炮塔角度一致
                    BulletLogic bulletLogic = bullet.GetComponent<BulletLogic>();
                    bulletLogic.speed = 100.0f;
                    break;
                }
            }

            /*
            if (bulletPrefab != null && bulletFolder != null && cannon != null && firePoint != null)
            {
                GameObject node = Entity.Instantiate(bulletPrefab, bulletFolder);  // 指定父节点，创建实例
                node.transform.position = this.firePoint.position;             // 指定出生点位置
                node.transform.eulerAngles = this.cannon.eulerAngles;          // 与炮塔角度一致
                BulletLogic bullet = node.GetComponent<BulletLogic>();
                bullet.speed = 10.0f;
            }
            */
            //node.transform.rotation = this.cannon.rotation;

            //BulletLogic script = node.GetComponent<BulletLogic>();

        }
    }
}
