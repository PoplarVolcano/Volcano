using System.Runtime.CompilerServices;

namespace Volcano
{
    public class Input
    {
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
            Input_GetMouseX(out float result);
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

    }
}