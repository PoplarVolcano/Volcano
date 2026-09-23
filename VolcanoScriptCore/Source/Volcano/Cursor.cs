
namespace Volcano
{
    public enum CursorLockMode
    {
        None,
        Locked,
        Confined
    }

    public class Cursor
    {
        public static CursorLockMode LockState
        {
            get
            {
                if (InternalCalls.MouseBuffer_GetMouseOnActive())
                    return CursorLockMode.None;
                else
                    return CursorLockMode.Locked;
            }
            set
            {
                if (value == CursorLockMode.None)
                {
                    InternalCalls.MouseBuffer_SetMouseOnActive(true);
                }
                else if (value == CursorLockMode.Locked)
                {
                    InternalCalls.MouseBuffer_SetMouseOnActive(false);
                }
            }
        }

        public static bool Visible { get; set; }
    }
}
