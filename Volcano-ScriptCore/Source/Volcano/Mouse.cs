using System;
using System.IO;

namespace Volcano
{
    public class Mouse
    {
        public static Mouse Instance
        {
            get
            {
                if (_instance == null)
                    _instance = new Mouse();
                return _instance;
            }
        }

        public bool FirstMouse { get; set; }
        public Vector2 MousePosition { get => Input.GetMousePosition(); }
        public float LastX { get; set; }
        public float LastY { get; set; }
        public bool OnActive 
        {
            get => InternalCalls.MouseBuffer_GetMouseOnActive();
            set => InternalCalls.MouseBuffer_SetMouseOnActive(ref value);
        }

        private Mouse()
        {
            LastX = 0.0f;
            LastY = 0.0f;
            FirstMouse = true;
            OnActive = true;
        }

        private static Mouse _instance = null;
    }
}
