#pragma once
#include "Volcano/Core/Base.h"

struct GLFWwindow;
namespace Volcano {
	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;

		static Ref<GraphicsContext> Create(GLFWwindow* windowHandle);
	};
}

