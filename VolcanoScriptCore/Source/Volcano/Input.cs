using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Volcano
{
    public class Input
    {
        public enum Axis
        {
            None,
            Horizontal,
            Vertical,
            MouseX,
            MouseY,
            Fire1,
            Fire2,
            Fire3,
            Jump,
            Submit,
            Cancel
        }

        private static Axis AxisFromString(string axis)
        {
            if (axis == "Horizontal") return Axis.Horizontal;
            if (axis == "Vertical")   return Axis.Vertical;
            if (axis == "Mouse X")     return Axis.MouseX;
            if (axis == "Mouse Y")     return Axis.MouseY;
            if (axis == "Fire1")      return Axis.Fire1;
            if (axis == "Fire2")      return Axis.Fire2;
            if (axis == "Fire3")      return Axis.Fire3;
            if (axis == "Jump")       return Axis.Jump;
            if (axis == "Submit")     return Axis.Submit;
            if (axis == "Cancel")     return Axis.Cancel;

            return Axis.None;
        }

        private static string AxisToString(Axis axis)
		{
			switch (axis)
			{
			case Axis.None:       return "None";
			case Axis.Horizontal: return "Horizontal";
			case Axis.Vertical:   return "Vertical";
			case Axis.MouseX:     return "Mouse X";
			case Axis.MouseY:     return "Mouse Y";
			case Axis.Fire1:      return "Fire1";
			case Axis.Fire2:      return "Fire2";
			case Axis.Fire3:      return "Fire3";
			case Axis.Jump:       return "Jump";
			case Axis.Submit:     return "Submit"; 
			case Axis.Cancel:     return "Cancel";
            default:
                return "None";
			}
		}

        /// <summary>鼠标当前的像素坐标位置（只读）</summary>
        public static Vector3 MousePosition { get; }

        /// <summary>指示是否检测到鼠标设备</summary>
        public static bool MousePresent { get; }

        /// <summary>当前的鼠标滚动增量（只读）</summary>
        public static Vector2 MouseScrollDelta { get; }

        /// <summary>当前是否有任何键或鼠标按钮处于按下状态？（只读）</summary>
        public static bool AnyKey { get; }

        /// <summary>在用户按下任意键或鼠标按钮后的第一帧返回 true（只读）</summary>
        public static bool AnyKeyDown { get; }

        /// <summary>启用/禁用通过触摸模拟鼠标操作（默认启用）—— 键鼠场景下通常保持为 true</summary>
        public static bool SimulateMouseWithTouches { get; set; }

        /// <summary>返回该帧输入的键盘输入（只读）</summary>
        public static string InputString { get; }


        // ======================== 静态方法 ========================

        /// <summary>获取虚拟轴的值（范围 -1...1）</summary>
        public static float GetAxis(string axisName)
        {
            switch(AxisFromString(axisName))
            {
                case Axis.Horizontal:
                    Input_GetHorizontal(out float horizontal);
                    return horizontal;
                case Axis.Vertical:
                    Input_GetVertical(out float vertical);
                    return vertical;
                case Axis.MouseX:
                    Input_GetMouseX(out float mouseX);
                    return mouseX;
                case Axis.MouseY:
                    Input_GetMouseY(out float mouseY);
                    return mouseY;
                default:
                    return 0;
            }
            return default;
        }

        /// <summary>获取虚拟轴的原始值（无平滑过滤）</summary>
        public static float GetAxisRaw(string axisName) { return default; }

        /// <summary>按住虚拟按钮时返回 true</summary>
        public static bool GetButton(string buttonName) { return default; }

        /// <summary>虚拟按钮被按下的第一帧返回 true</summary>
        public static bool GetButtonDown(string buttonName) { return default; }

        /// <summary>虚拟按钮被释放的第一帧返回 true</summary>
        public static bool GetButtonUp(string buttonName) { return default; }

        /// <summary>按住指定键时返回 true（通过 KeyCode）</summary>
        public static bool GetKey(KeyCode key) {
            return Input_IsKeyPressed(key);
        }

        /// <summary>按住指定键时返回 true（通过键名）</summary>
        public static bool GetKey(string name) { return default; }

        /// <summary>指定键被按下的第一帧返回 true（通过 KeyCode）</summary>
        public static bool GetKeyDown(KeyCode key) { return default; }

        /// <summary>指定键被按下的第一帧返回 true（通过键名）</summary>
        public static bool GetKeyDown(string name) { return default; }

        /// <summary>指定键被释放的第一帧返回 true（通过 KeyCode）</summary>
        public static bool GetKeyUp(KeyCode key) { return default; }

        /// <summary>指定键被释放的第一帧返回 true（通过键名）</summary>
        public static bool GetKeyUp(string name) { return default; }

        /// <summary>返回指定鼠标按钮是否被按住（0=左键，1=右键，2=中键）</summary>
        public static bool GetMouseButton(int button) { return default; }

        /// <summary>返回指定鼠标按钮被按下的第一帧</summary>
        public static bool GetMouseButtonDown(int button) { return default; }

        /// <summary>返回指定鼠标按钮被释放的第一帧</summary>
        public static bool GetMouseButtonUp(int button) { return default; }

        /// <summary>重置输入轴。重置后，所有轴都恢复到中性状态</summary>
        public static void ResetInputAxes() { }


        public static bool IsKeyClicked(KeyCode keycode)
        {
            return Input_IsKeyClicked(keycode);
        }

        public static bool IsMouseButtonClicked(MouseCode mouseButton)
        {
            return Input_IsMouseButtonClicked(mouseButton);
        }

        public static bool IsKeyPressed(KeyCode keycode)
        {
            return Input_IsKeyPressed(keycode);
        }

        public static bool IsMouseButtonPressed(MouseCode mouseButton)
        {
            return Input_IsMouseButtonPressed(mouseButton);
        }

        public static Vector2 GetMousePosition()
        {
            Input_GetMousePosition(out Vector2 result);
            return result;
        }

        public static float GetMouseX()
        {
            Input_GetMouseX(out float result);
            return result;
        }

        public static float GetMouseY()
        {
            Input_GetMouseY(out float result);
            return result;
        }


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyClicked(KeyCode keycode);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseButtonClicked(MouseCode mouseButton);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyPressed(KeyCode keycode);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseButtonPressed(MouseCode mouseButton);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMousePosition(out Vector2 mousePosition);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMouseX(out float mouseX);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMouseY(out float mouseY);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetHorizontal(out float horizontal);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetVertical(out float vertical);

    }
}