#pragma once

#include "Volcano/Core/Core.h"

#include <imgui.h>

namespace Volcano {

    class VOL_API ImGuiContextManager 
    {
    public:
        static void SetContext(ImGuiContext* context);

        static ImGuiContext* GetContext();

        static void MakeCurrent();

        static void ShowDemoWindow(bool* p_open = NULL);
    private:
        static ImGuiContext* s_Context;
    };

}