#include "volpch.h"
#include "ImGuiContextManager.h"

namespace Volcano 
{
    ImGuiContext* ImGuiContextManager::s_Context = nullptr;

    void ImGuiContextManager::SetContext(ImGuiContext* context) 
    {
        s_Context = context;
    }

    ImGuiContext* ImGuiContextManager::GetContext() 
    {
        return s_Context;
    }

    void ImGuiContextManager::MakeCurrent()
    {
        if (s_Context) {
            ImGui::SetCurrentContext(s_Context);
        }
    }

    void ImGuiContextManager::ShowDemoWindow(bool* p_open)
    {
        if (ImGui::GetCurrentContext())
        {
            ImGui::ShowDemoWindow(p_open);
        }
    }
}