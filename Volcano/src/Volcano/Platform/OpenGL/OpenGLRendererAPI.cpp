#include "volpch.h"

#include "Volcano/Renderer/RendererAPI.h"
#include <glad/glad.h>

namespace Volcano {
	
	static GLenum VolcanoToOpenGLDepthFunc(DepthFunc func)
	{
		switch (func)
		{
		    case DepthFunc::NEVER:    return GL_NEVER;
		    case DepthFunc::LESS:     return GL_LESS;
		    case DepthFunc::EQUAL:    return GL_EQUAL;
		    case DepthFunc::LEQUAL:   return GL_LEQUAL;
		    case DepthFunc::GREATER:  return GL_GREATER;
		    case DepthFunc::NOTEQUAL: return GL_NOTEQUAL;
		    case DepthFunc::GEQUAL:   return GL_GEQUAL;
		    case DepthFunc::ALWAYS:   return GL_ALWAYS;
		}
		VOL_CORE_ASSERT(false, "Unknown texture format!");
		return 0;
	}

	static GLenum VolcanoToOpenGLCullFaceFunc(CullFaceFunc func)
	{
		switch (func)
		{
		    case CullFaceFunc::BACK:           return GL_BACK;
		    case CullFaceFunc::FRONT:          return GL_FRONT;
		    case CullFaceFunc::FRONT_AND_BACK: return GL_FRONT_AND_BACK;
		}
		VOL_CORE_ASSERT(false, "Unknown texture format!");
		return 0;
	}

	void RendererAPI::Init()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		auto& caps = RendererAPI::GetCapabilities();

		caps.Vendor = (const char*)glGetString(GL_VENDOR);
		caps.Renderer = (const char*)glGetString(GL_RENDERER);
		caps.Version = (const char*)glGetString(GL_VERSION);

		glGetIntegerv(GL_MAX_SAMPLES, &caps.MaxSamples);
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &caps.MaxAnisotropy);
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &caps.MaxTextureUnits);

	}

	void RendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void RendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void RendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void RendererAPI::SetClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void RendererAPI::Clear(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount, bool depthTest)
	{
		if (!depthTest)
			glDisable(GL_DEPTH_TEST);

		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		//如何绘制索引， 多少个索引， 索引类型， 偏移量
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);

		if (!depthTest)
			glEnable(GL_DEPTH_TEST);
	}

	void RendererAPI::DrawArrays(const Ref<VertexArray>& vertexArray, uint32_t count, bool depthTest)
	{
		if (!depthTest)
			glDisable(GL_DEPTH_TEST);

		vertexArray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, count);// 绘制的模式、起始顶点索引和顶点数量。

		if (!depthTest)
			glEnable(GL_DEPTH_TEST);
	}

	void RendererAPI::DrawStripIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount, bool depthTest)
	{
		if (!depthTest)
			glDisable(GL_DEPTH_TEST);

		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		//如何绘制索引， 多少个索引， 索引类型， 偏移量
	    glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, nullptr);

		if (!depthTest)
			glEnable(GL_DEPTH_TEST);
	}

	void RendererAPI::DrawInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t amount, bool depthTest)
	{
		if (!depthTest)
			glDisable(GL_DEPTH_TEST);

		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		//如何绘制索引， 多少个索引， 索引类型， 偏移量
		glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0, amount);
		vertexArray->UnBind();

		if (!depthTest)
			glEnable(GL_DEPTH_TEST);
	}

	void RendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void RendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}

	void RendererAPI::SetDepthTest(bool depthTest)
	{
		if(depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	void RendererAPI::SetCullFace(bool cullFace)
	{
		if (cullFace)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}

	void RendererAPI::SetDepthFunc(DepthFunc func)
	{
		glDepthFunc(VolcanoToOpenGLDepthFunc(func));
	}

	void RendererAPI::SetCullFaceFunc(CullFaceFunc func)
	{
		glCullFace(VolcanoToOpenGLCullFaceFunc(func));
	}

	void RendererAPI::SetPolygonMode(bool type)
	{
		// 使用线框模式绘制
		if(type)
		    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
