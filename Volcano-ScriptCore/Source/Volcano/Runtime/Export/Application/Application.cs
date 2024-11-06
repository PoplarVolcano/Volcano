using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public class Application
    {

        // Instructs game to try to render at a specified frame rate.
        public static int targetFrameRate
        {
            get { InternalCalls.Application_GetTargetFrameRate(out int result); return result; }
            set { InternalCalls.Application_SetTargetFrameRate(value); }
        }

    }
}
