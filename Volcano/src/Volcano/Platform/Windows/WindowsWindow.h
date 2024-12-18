#pragma once

#include "Volcano/Core/Window.h"
#include "Volcano/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace Volcano {

    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(const WindowProps& props);
        virtual ~WindowsWindow();

        void OnUpdate() override;

        inline uint32_t GetWidth() const override { return m_Data.Width; }
        inline uint32_t GetHeight() const override { return m_Data.Height; }


        // Window attributes
        inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
        void SetVSync(bool enabled) override;
        bool IsVSync() const override;
        
        inline virtual void* GetNativeWindow() const { return m_Window; }
        virtual void SetMouseActive(bool mouseActive) override;
        virtual void SetViewport(float width, float height) override;
        virtual void ResetViewport() override;
        virtual void ClearConsole() override;
    private:
        virtual void Init(const WindowProps& props);
        virtual void Shutdown();
    private:
        GLFWwindow* m_Window;
        Ref<GraphicsContext> m_Context;
        GLFWcursor* m_ImGuiMouseCursors[9] = { 0 };

        struct WindowData
        {
            std::string Title;
            uint32_t Width, Height;
            bool VSync;

            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };

}