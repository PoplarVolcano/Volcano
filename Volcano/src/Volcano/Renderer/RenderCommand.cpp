#include "volpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Volcano {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;

}