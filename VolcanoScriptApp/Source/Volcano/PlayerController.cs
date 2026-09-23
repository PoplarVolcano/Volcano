
using System;
using System.Threading;
using static Volcano.Input;

namespace Volcano
{
    public class PlayerController : Volcano.MonoBehaviour
    {
        public string Message { get; set; }

        public Transform CameraTransform;
        public float WalkSpeed = 5.0f;
        public float MouseSensitivity = 50.0f;
        public float RollSpeed = 90.0f;

        private float m_LastX = 0.0f, m_LastY = 0.0f;
        private float m_Yaw = 0.0f;
        private float m_Pitch = 0.0f;
        private float m_Roll = 0.0f;

        public void Reset()
        {
            Message = "PlayerController Reset";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
            MouseSensitivity = 100.0f;
        }

        public void Awake()
        {
            Message = "PlayerController Awake";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }

        public void OnEnable()
        {
            Message = "PlayerController OnEnable";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }

        public void Start()
        {
            Message = "PlayerController Start";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");

            Cursor.LockState = CursorLockMode.Locked;
            m_LastX = Input.GetAxis("Mouse X");
            m_LastY = Input.GetAxis("Mouse Y");

            // 初始化朝向
            Vector3 e = Transform.Rotation.eulerAngles;
            m_Pitch = e.x;
            m_Yaw   = e.y;
            m_Roll  = e.z;

            // CameraTransform在C++端被赋值了一个Transform实例，ID为目标的entityID
            // 但是CameraTransform == null返回true，所以需要用ID != 0判断是否赋值
            // 让player和camera的世界空间坐标和方向保持一致
            if (CameraTransform.GetInstanceID() != 0)
            {
                CameraTransform.Position = Transform.Position;
                CameraTransform.Rotation = Transform.Rotation;
            }
        }

        public void Update()
        {
            //Message = "PlayerController Update";
            //Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");

            if (Input.GetKey(KeyCode.LeftAlt))
            {
                Cursor.LockState = CursorLockMode.None;
                return;
            }

            Cursor.LockState = CursorLockMode.Locked;

            HandleMouseLook();
            HandleMovement();
        }

        private void HandleMouseLook()
        {
            // 获取鼠标输入
            float deltaX = Input.GetAxis("Mouse X") - m_LastX;
            float deltaY = Input.GetAxis("Mouse Y") - m_LastY;
            m_LastX = Input.GetAxis("Mouse X");
            m_LastY = Input.GetAxis("Mouse Y");

            m_Yaw   -= deltaX * MouseSensitivity;
            m_Pitch += deltaY * MouseSensitivity;

            if (Input.GetKey(KeyCode.Q)) m_Roll += RollSpeed * Time.DeltaTime;
            if (Input.GetKey(KeyCode.E)) m_Roll -= RollSpeed * Time.DeltaTime;

            // 限制俯仰角，防止翻转
            //m_Pitch = Math.Clamp(m_Pitch, -89.0f, 89.0f);

            // 相机：先世界 Y 偏航，再局部 X 俯仰
            Quaternion rollRototation = Quaternion.AngleAxis(m_Roll, Vector3.Forward);

            // 注：Vector3.Forward 是世界 Z 轴。
            // 如果想"绕相机的 Z 轴"旋转，应该用局部轴
            // 但 AngleAxis(m_Roll, Vector3.Forward) 与 Euler(pitch, yaw, 0) 左乘时，
            // 实际就是绕"Euler 旋转之后物体的局部 Z 轴"（因为 q1 * q2 的 q2 是在 q1 的局部坐标系下作用的）
            Transform.Rotation = Quaternion.Euler(m_Pitch, m_Yaw, 0.0f) * rollRototation;

            if (CameraTransform.GetInstanceID() != 0)
            {
                CameraTransform.Rotation = Transform.Rotation;
            }
        }

        private void HandleMovement()
        {
            float horizontal = 0.0f;
            float vertical = 0.0f;
            // 获取WASD输入
            if (Input.GetKey(KeyCode.A)) horizontal -= 1.0f;
            if (Input.GetKey(KeyCode.D)) horizontal += 1.0f;
            if (Input.GetKey(KeyCode.W)) vertical   += 1.0f;
            if (Input.GetKey(KeyCode.S)) vertical   -= 1.0f;

            float upDown = 0.0f;
            if (Input.GetKey(KeyCode.Space))
            {
                upDown += 1.0f;
            }
            if (Input.GetKey(KeyCode.LeftControl)|| Input.GetKey(KeyCode.RightControl))
            {
                upDown -= 1.0f;
            }

            if (horizontal == 0.0f && vertical == 0.0f && upDown == 0.0f)
                return;

            Vector3 forward = Transform.Rotation * Vector3.Forward;
            Vector3 right = Transform.Rotation * Vector3.Right;
            forward.y = 0.0f;
            right.y = 0.0f;
            forward.Normalize();
            right.Normalize();

            Vector3 moveDirection = forward * vertical + right * horizontal + Vector3.Up * upDown;
            if (moveDirection != Vector3.Zero)
                moveDirection.Normalize();
            Vector3 move = moveDirection * WalkSpeed * Time.DeltaTime;

            Transform.Position += move;
            if (CameraTransform.GetInstanceID() != 0)
            {
                CameraTransform.Position = Transform.Position;
            }

        }

        public void FixedUpdate()
        {
            //Message = "PlayerController FixedUpdate";
            //Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }

        public void LateUpdate()
        {
            //Message = "PlayerController LateUpdate";
            //Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }

        public void OnDisable()
        {
            Message = "PlayerController OnDisable";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }

        public void OnDestroy()
        {
            Message = "PlayerController OnDestroy";
            Volcano.InternalCalls.Debug_Trace($"C# says: {Message}");
        }
    }
}
