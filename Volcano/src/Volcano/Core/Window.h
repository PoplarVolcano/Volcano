#pragma once

#include "Volcano/Core/Base.h"
#include "Volcano/Core/Events/Event.h"

namespace Volcano {

    struct WindowProps//窗口属性
    {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowProps(const std::string& title = "Volcano Engine",
            uint32_t width = 1600,
            uint32_t height = 900)
            : Title(title), Width(width), Height(height)
        {
        }
    };

    // Interface representing a desktop system based Window
    class Window { //窗口抽象类

    public:
        // 表示接受一个参数类型为 Event&(Event指针)，返回类型为 void 的函数对象。
        // std::function 是一个通用的函数封装类，用于封装可调用对象，如函数指针、成员函数指针、Lambda 表达式等。
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() {}

        virtual void OnUpdate() = 0;//每一帧调用

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        // Window attributes
        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;//设置窗口事件回调,平台触发
        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;

        virtual void* GetNativeWindow() const = 0;

        virtual void SetMouseActive(bool mouseActive) = 0;
        virtual void SetViewport(float width, float height) = 0;
        virtual void ResetViewport() = 0;
        static Scope<Window> Create(const WindowProps& props = WindowProps());
    };

}