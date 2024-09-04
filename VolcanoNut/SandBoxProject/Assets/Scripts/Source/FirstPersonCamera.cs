namespace Volcano
{
    public class FirstPersonCamera : Entity
    {
        public float distance = 3.0f; // 摄像头与玩家之间的初始距离
        public float maxDistance = 10.0f; // 摄像头与玩家之间的最大距离
        public float rotationSpeed = 3.0f; // 摄像头旋转速度

        private TransformComponent playerTransform; // 玩家的变换组件
        private float currentDistance; // 当前摄像头距离

        void Start()
        {
            playerTransform = transform; // 获取玩家的变换组件
            currentDistance = distance; // 初始化当前距离

            // 设置摄像头初始位置和目标
            Vector3 desiredPosition = playerTransform.T + playerTransform.forward * currentDistance;
            transform.position = desiredPosition;
            transform.LookAt(playerTransform);
        }

        void Update()
        {
            // 旋转摄像头
            float horizontal = Input.GetAxis("Mouse X") * rotationSpeed;
            float vertical = Input.GetAxis("Mouse Y") * rotationSpeed;

            playerTransform.Rotate(0, horizontal, 0);

            // 限制最大最小距离
            currentDistance -= Input.GetAxis("Mouse ScrollWheel") * 5;
            currentDistance = Mathf.Clamp(currentDistance, 0.1f, maxDistance);

            // 更新摄像头位置
            transform.position = playerTransform.position + playerTransform.forward * currentDistance;
            transform.LookAt(playerTransform);
        }
    }
}
