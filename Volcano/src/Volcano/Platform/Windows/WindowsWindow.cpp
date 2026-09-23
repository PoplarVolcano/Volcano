#include "volpch.h"
#include <glad/glad.h>
#include "WindowsWindow.h"

#include "Volcano/Core/Events/ApplicationEvent.h"
#include "Volcano/Core/Events/KeyEvent.h"
#include "Volcano/Core/Events/MouseEvent.h"

namespace Volcano {

    static void GLFWErrorCallback(int error_code, const char* description) {
        VOL_CORE_ERROR("GLFW Error ({0}):{1}", error_code, description);
    }

    static bool s_GLFWInitialized = false;

    WindowsWindow::WindowsWindow(const WindowProperties& props)
    {
        Init(props);
    }

    WindowsWindow::~WindowsWindow()
    {
        Shutdown();
    }

    void WindowsWindow::SetVSync(bool enabled)
    {
        // 控制垂直同步(V-Sync),决定在调用glfwSwapBuffers交换前后缓冲区时，程序需要等待多少个屏幕刷新周期
        // 调用此函数前，当前线程必须有一个激活的OpenGL或OpenGL ES上下文，否则会触发GLFW_NO_CURRENT_CONTEXT错误
        // 在使用Vulkan进行渲染，这个函数是无效的。
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_Data.VSync = enabled;
    }

    bool WindowsWindow::IsVSync() const
    {
        return m_Data.VSync;
    }

    void WindowsWindow::SetMouseActive(bool mouseOnActive)
    {
        if(mouseOnActive)
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void WindowsWindow::SetViewport(float width, float height)
    {
        glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    }

    void WindowsWindow::ResetViewport()
    {
        glViewport(0, 0, m_Data.Width, m_Data.Height);
    }

    void WindowsWindow::ClearConsole()
    {
        system("cls");
    }

    void WindowsWindow::Init(const WindowProperties& props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        VOL_CORE_INFO("Creating window {0} ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height);
        
        if (!s_GLFWInitialized)
        {
            int success = glfwInit();
            VOL_CORE_ASSERT(success, "Could not intialize GLFW!");
            glfwSetErrorCallback(GLFWErrorCallback);

            s_GLFWInitialized = true;
        }

        // glfwWindowHint 在 glfwCreateWindow 之前调用，且通常在 glfwInit 之后。
        // 这个设置只影响 GLFW 创建的默认窗口帧缓冲（default framebuffer）
        // 现在主要使用自定义帧缓冲FBO，所以没用上。
        glfwWindowHint(GLFW_SAMPLES, 4); // 请求4x MSAA

        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(m_Window);
        //glfwMaximizeWindow(m_Window);//窗口最大化

        //初始化OpenGL上下文
        m_Context = GraphicsContext::Create(m_Window);
        m_Context->Init();

        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(true);

        // 设置窗口尺寸回调函数，参数GLFWwindowsizefun为(GLFWwindow* window, int width, int height)为参数的lambda表达式
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            //获取初始化窗口时输入的WindowData类型的m_Data
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            WindowResizeEvent event((unsigned int)width, (unsigned int)height);
            // EventCallback的类型是事件回调函数EventCallbackFn = std::function<void(Event&)>
            // 将m_Data的EventCallback设置为event
            // 触发事件时，创建对应Event对象(如WindowResizeEvent)作为参数调用EventCallback=>Application.OnEvent()=>LayerStack.OnEvent()
            data.EventCallback(event);
            data.Width = width;
            data.Height = height;
            });

        // 设置窗口关闭回调函数
        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            WindowCloseEvent event;
            data.EventCallback(event);
            });

        // 设置键盘回调函数
        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action) {
            case GLFW_PRESS: {
                KeyPressedEvent event(key, 0);
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event(key);
                data.EventCallback(event);
                break;
            }
            case GLFW_REPEAT: {
                KeyPressedEvent event(key, true);
                data.EventCallback(event);
                break;
            }
            }
            });

        // 设置文本输入回调函数
        // 用于处理文本输入，用于获取用户通过键盘输入的字符
        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            KeyTypedEvent event(codepoint);

            data.EventCallback(event);
            });

        // 设置鼠标按键回调函数
        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent event(button);
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent event(button);
                data.EventCallback(event);
                break;
            }
            }
            });

        // 设置鼠标滚轮回调函数
        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.EventCallback(event);
            });

        // 鼠标光标在指定窗口内移动时触发，报告光标的实时位置
        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float)xPos, (float)yPos);
            data.EventCallback(event);
            });


    }

    void WindowsWindow::Shutdown()
    {
        glfwDestroyWindow(m_Window);
    }

    void WindowsWindow::OnUpdate()
    {
        //轮询事件
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
    }

}