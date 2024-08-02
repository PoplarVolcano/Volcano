#pragma once

#include "Volcano/Renderer/GraphicsContext.h"
#include "Volcano/Log.h"

struct GLFWwindow;
namespace Volcano {
	class OpenGLContext : public GraphicsContext 
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);
		virtual void Init() override;
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}