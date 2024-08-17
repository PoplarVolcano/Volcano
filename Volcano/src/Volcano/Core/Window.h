#pragma once

#include "Volcano/Core/Base.h"
#include "Volcano/Core/Events/Event.h"

namespace Volcano {

    struct WindowProps//��������
    {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowProps(const std::string& title = "Volcano Engine",
            uint32_t width = 1280,
            uint32_t height = 720)
            : Title(title), Width(width), Height(height)
        {
        }
    };

    // Interface representing a desktop system based Window
    class Window { //���ڳ�����

    public:
        // ��ʾ����һ����������Ϊ Event&(Eventָ��)����������Ϊ void �ĺ�������
        // std::function ��һ��ͨ�õĺ�����װ�࣬���ڷ�װ�ɵ��ö����纯��ָ�롢��Ա����ָ�롢Lambda ���ʽ�ȡ�
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() {}

        virtual void OnUpdate() = 0;//ÿһ֡����

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual std::pair<uint32_t, uint32_t> GetSize() const = 0;
        virtual std::pair<float, float> GetWindowPos() const = 0;

        // Window attributes
        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;//���ô����¼��ص�,ƽ̨����
        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;

        virtual void* GetNativeWindow() const = 0;

        virtual float GetTime() const = 0;

        static Window* Create(const WindowProps& props = WindowProps());
    };

}