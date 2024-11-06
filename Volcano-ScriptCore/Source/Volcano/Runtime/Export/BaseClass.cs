using System;

namespace Volcano
{
    // The coordinate space in which to operate.
    public enum Space
    {
        // Applies transformation relative to the world coordinate system
        World = 0,
        // Applies transformation relative to the local coordinate system
        Self = 1
    }

}
