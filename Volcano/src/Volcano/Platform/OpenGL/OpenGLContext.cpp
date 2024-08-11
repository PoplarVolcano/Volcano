#include "volpch.h"
#include "OpenGLContext.h"

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<gl/GL.h>

namespace Volcano {
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:m_WindowHandle(windowHandle)
	{
		VOL_CORE_ASSERT(windowHandle, "windowHandle为空");
	}
	void OpenGLContext::Init()
	{	
		//在GLFW中绑定OpenGL上下文
		glfwMakeContextCurrent(m_WindowHandle);
		//在运行时获取OpenGL函数地址并将其保存在函数指针中供以后使用
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		VOL_CORE_ASSERT(status, "glad初始化错误");

		VOL_CORE_INFO("OpenGL Info:");
		VOL_CORE_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		VOL_CORE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
		VOL_CORE_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));
	}
	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}