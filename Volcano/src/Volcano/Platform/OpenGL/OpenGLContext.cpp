#include "volpch.h"
#include "OpenGLContext.h"

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<gl/GL.h>

namespace Volcano {
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:m_WindowHandle(windowHandle)
	{
		VOL_CORE_ASSERT(windowHandle, "windowHandleΪ��");
	}
	void OpenGLContext::Init()
	{	
		//��GLFW�а�OpenGL������
		glfwMakeContextCurrent(m_WindowHandle);
		//������ʱ��ȡOpenGL������ַ�����䱣���ں���ָ���й��Ժ�ʹ��
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		VOL_CORE_ASSERT(status, "glad��ʼ������");

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