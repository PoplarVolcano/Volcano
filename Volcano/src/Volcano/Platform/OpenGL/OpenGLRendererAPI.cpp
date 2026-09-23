#include "volpch.h"

#include "Volcano/Renderer/RendererAPI.h"
#include <glad/glad.h>

namespace Volcano {

	static GLenum VolcanoToOpenGLStencilOp(StencilOp op)
	{
		switch (op)
		{
		case StencilOp::KEEP:    return GL_KEEP;
		case StencilOp::ZERO:    return GL_ZERO;
		case StencilOp::REPLACE: return GL_REPLACE;
		case StencilOp::INCR:    return GL_INCR;
		case StencilOp::DECR:    return GL_DECR;
		case StencilOp::INVERT:  return GL_INVERT;
		default:
			return 0;
		}
		VOL_CORE_ASSERT(false, "Unknown StencilOp!");
	}

	static GLenum VolcanoToOpenGLBlendFunc(BlendFunc func)
	{
		switch (func)
		{
		case BlendFunc::SRC_ALPHA:           return GL_SRC_ALPHA;
		case BlendFunc::ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFunc::ONE:                 return GL_ONE;
		case BlendFunc::ZERO:                return GL_ZERO;
		case BlendFunc::DST_COLOR:           return GL_DST_COLOR;
		default:
			return 0;
		}
		VOL_CORE_ASSERT(false, "Unknown BlendFunc!");
	}

	static BlendFunc OpenGLBlendFuncToVolcano(GLenum func)
	{
		switch (func)
		{
		case GL_SRC_ALPHA:               return BlendFunc::SRC_ALPHA;
		case GL_ONE_MINUS_SRC_ALPHA:	 return BlendFunc::ONE_MINUS_SRC_ALPHA;
		case GL_ONE:					 return BlendFunc::ONE;
		case GL_ZERO:					 return BlendFunc::ZERO;
		case GL_DST_COLOR:				 return BlendFunc::DST_COLOR;
		default:
			return BlendFunc::SRC_ALPHA;
		}
		VOL_CORE_ASSERT(false, "Unknown BlendFunc!");
	}

	void RendererAPI::Init()
	{
		// 线条抗锯齿：
		// 默认关闭，让绘制线条时边缘出现锯齿状像素得到平滑处理（通过混合（Blending）技术）
		// 须同时调用glEnable(GL_BLEND)和设置合适的混合函数如glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)才能生效
		// 现代OpenGL中，更推荐使用MSAA（多重采样抗锯齿） 来处理几何边缘
		// glEnable(GL_LINE_SMOOTH);

		glEnable(GL_MULTISAMPLE);

		// 无缝立方体贴图采样：
		// 默认关闭，在采样立方体贴图（Cube Map，常用于天空盒、环境反射、折射效果）时，
		// 消除立方体六个面之间的接缝痕迹。当纹理坐标恰好落在两个面的边界附近时，不开启该功能可能会导致接缝处出现明显的黑色线条或纹理错位。
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// 查询 OpenGL 渲染器的硬件能力上限
		auto& caps = RendererAPI::GetCapabilities();

		caps.Vendor = (const char*)glGetString(GL_VENDOR);
		caps.Renderer = (const char*)glGetString(GL_RENDERER);
		caps.Version = (const char*)glGetString(GL_VERSION);

		// 显卡支持的最高多重采样抗锯齿（MSAA）级别，即每个像素最多可以包含的样本数量
		// 创建多采样渲染缓冲（Renderbuffer）或多采样纹理时，需指定样本数（Samples）≤ caps.MaxSamples，否则 OpenGL 会报错（GL_INVALID_OPERATION）。
		glGetIntegerv(GL_MAX_SAMPLES, &caps.MaxSamples);

		// 显卡支持的各向异性过滤（Anisotropic Filtering）的最高等级，用于改善倾斜视角下纹理的清晰度
		// 要使用各向异性过滤，须通过这个函数查询硬件支持的最高等级，
		// 然后调用 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, level) 来设置。
		// 传入的值必须 ≤ caps.MaxAnisotropy
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &caps.MaxAnisotropy);

		// 着色器（Shader）中最多能同时使用多少个纹理单元（Texture Units）。
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &caps.MaxTextureUnits);

	}

	void RendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void RendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void RendererAPI::ClearStencil()
	{
		glClear(GL_STENCIL_BUFFER_BIT);
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

	void RendererAPI::ClearStencil(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	void RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		//如何绘制索引， 多少个索引， 索引类型， 偏移量
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void RendererAPI::DrawArrays(const Ref<VertexArray>& vertexArray, uint32_t count)
	{
		vertexArray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, count);// 绘制的模式、起始顶点索引和顶点数量。

	}

	void RendererAPI::DrawStripIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		//如何绘制索引， 多少个索引， 索引类型， 偏移量
		glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, nullptr);
	}

	void RendererAPI::DrawIndexInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t amount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0, amount);
		vertexArray->UnBind();
	}

	void RendererAPI::DrawStripIndexInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t amount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElementsInstanced(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, 0, amount);
		vertexArray->UnBind();
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

	void RendererAPI::SetCullFaceEnable(bool enable)
	{
		// 面剔除：默认关闭，根据三角形的顶点绕序（顺时针或逆时针）决定是否丢弃（不渲染）某个三角形。
		if (enable)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}

	void RendererAPI::SetCullFaceFunc(CullFaceFunc func)
	{
		// glCullFace(GL_BACK)：设置剔除背面（默认，也是最常用的）。
		switch (func)
		{
		case CullFaceFunc::BACK:           glCullFace(GL_BACK);           break;
		case CullFaceFunc::FRONT:          glCullFace(GL_FRONT);          break;
		case CullFaceFunc::FRONT_AND_BACK: glCullFace(GL_FRONT_AND_BACK); break;
		default:
			break;
		}
	}

	void RendererAPI::SetCullFaceFrontFace(CullFaceFrontFace frontFace)
	{
		// glFrontFace(GL_CCW)：设置逆时针（Counter - Clockwise）为正面（默认）。
		switch (frontFace)
		{
		case CullFaceFrontFace::CCW: glFrontFace(GL_CCW); break;
		case CullFaceFrontFace::CW:  glFrontFace(GL_CW);  break;
		default:
			break;
		}
	}

	void RendererAPI::SetPolygonMode(PolygonMode polygonMode)
	{
		switch (polygonMode)
		{
		case PolygonMode::POINT: glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
		case PolygonMode::LINE:  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  break;
		case PolygonMode::FILL:  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  break;
		default:
			break;
		}
	}

	void RendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		// x, y：指定视口矩形的左下角在窗口坐标系中的位置（以像素为单位）。窗口的原点 (0,0) 位于左下角（与纹理坐标不同，要注意区分）。
		// width, height：指定视口矩形的宽度和高度（以像素为单位）
		glViewport(x, y, width, height);
	}

	void RendererAPI::SetMultisampleEnable(bool enable)
	{
		// 帧缓冲必须包含多采样附件, 默认开启（如果创建了多采样 FBO）
		if (enable)
			glEnable(GL_MULTISAMPLE);
		else
			glDisable(GL_MULTISAMPLE);
	}

	void RendererAPI::SetStencilTestEnable(bool enable)
	{
		// 模板测试：
		// 默认关闭，使用模板缓冲区（Stencil Buffer）来限制绘制区域。
		// 可以先在模板缓冲区中绘制一个形状（例如一个圆形或人物轮廓），然后后续的绘制操作只会在模板值匹配的区域进行绘制。
		// 配套函数glStencilFunc、glStencilOp 控制测试条件和缓冲区更新规则。
		if (enable)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}

	void RendererAPI::SetStencilFunc(StencilFunc func, int ref, uint32_t mask)
	{
		// 设置模板测试比较规则：当前片元的模板值 vs 缓冲区中的模板值，是否通过测试
		// 你的值（ref）和库里的值（buffer）比较，条件满足才通过。

		// glStencilFunc(GLenum func, GLint ref, GLuint mask)包含三个参数：
		// 
		// func：模板测试函数(Stencil Test Function)。
		// 这个测试函数将会应用到已储存的模板值和glStencilFunc函数的ref值上。
		// 可用的选项有：GL_NEVER、GL_LESS、GL_LEQUAL、GL_GREATER、GL_GEQUAL、GL_EQUAL、GL_NOTEQUAL和GL_ALWAYS。它们的语义和深度缓冲的函数类似。
		// 
		// ref：设置了模板测试的参考值(Reference Value)。模板缓冲的内容将会与这个值进行比较。
		// 
		// mask：设置一个掩码，它将会与参考值和储存的模板值在测试比较它们之前进行与(AND)运算。初始情况下所有位都为1。
		// 注：
		// SetStencilFunc的mask 在读取/比较时使用，决定读取模板缓冲区时，哪些位参与比较（ref & mask 对比 buffer & mask）
		// glStencilMask 的mask 在写入时使用，决定写入模板缓冲区时，哪些位可以被修改（0xFF 全开，0x00 全关）

		if ((GLenum)func >= GL_NEVER)
		{
			glStencilFunc((GLenum)func, ref, mask);
			return;
		}

		switch (func)
		{
		case StencilFunc::NEVER:    glStencilFunc(GL_NEVER,    ref, mask); break;
		case StencilFunc::LESS:     glStencilFunc(GL_LESS,     ref, mask); break;
		case StencilFunc::EQUAL:    glStencilFunc(GL_EQUAL,    ref, mask); break;
		case StencilFunc::LEQUAL:   glStencilFunc(GL_LEQUAL,   ref, mask); break;
		case StencilFunc::GREATER:  glStencilFunc(GL_GREATER,  ref, mask); break;
		case StencilFunc::NOTEQUAL: glStencilFunc(GL_NOTEQUAL, ref, mask); break;
		case StencilFunc::GEQUAL:   glStencilFunc(GL_GEQUAL,   ref, mask); break;
		case StencilFunc::ALWAYS:   glStencilFunc(GL_ALWAYS,   ref, mask); break;
		default:
			break;
		}
	}

	void RendererAPI::SetStencilOp(StencilOp sfail, StencilOp dpfail, StencilOp dppass)
	{
		// 更新规则：测试通过/失败后，如何修改缓冲区的模板值
		// 通过后把库里的值（buffer）改成你带的值（ref）（REPLACE），或者不改（KEEP）
		// 
		// glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)包含三个选项，能够设定每个选项应该采取的行为：
		// 
		// sfail：模板测试失败时采取的行为。
		// 
		// dpfail：模板测试通过，但深度测试失败时采取的行为。
		// 
		// dppass：模板测试和深度测试都通过时采取的行为。
		// 
		// 默认情况下glStencilOp是设置为(GL_KEEP, GL_KEEP, GL_KEEP)的，不论任何测试的结果是如何，模板缓冲都会保留它的值。

		GLenum glSfail  = VolcanoToOpenGLStencilOp(sfail);
		GLenum glDpfail = VolcanoToOpenGLStencilOp(dpfail);
		GLenum glDppass = VolcanoToOpenGLStencilOp(dppass);
		if (glSfail != 0 && glDpfail != 0 && glDppass != 0)
		    glStencilOp(glSfail, glDpfail, glDppass);
	}

	void RendererAPI::SetStencilMask(uint32_t mask)
	{
		// glStencilMask允许我们设置一个位掩码(Bitmask)，它会与将要写入缓冲的模板值进行与(AND)运算。
		// 默认情况下设置的位掩码所有位都为1，不影响输出，但如果我们将它设置为0x00，写入缓冲的所有模板值最后都会变成0
		// glStencilMask(0xFF); // 每一位写入模板缓冲时都保持原样
		// glStencilMask(0x00); // 每一位在写入模板缓冲时都会变成0（禁用写入）

		glStencilMask(mask);
	}

	void RendererAPI::SetDepthTestEnable(bool enable)
	{
		// 深度测试：深度缓冲区（Z-buffer）测试，
		// 当绘制每个像素时，OpenGL会比较该片元的深度值与深度缓冲区中已存储的值，只有通过测试的片元才会被绘制。
		// 没有它，所有物体都会按照绘制顺序叠加，后绘制的会覆盖先绘制的，即使它实际上在场景中更靠后。

		if (enable)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	void RendererAPI::SetDepthTestFunc(DepthTestFunc func)
	{
		if ((GLenum)func >= GL_NEVER)
		{
			glDepthFunc((GLenum)func);
			return;
		}

		// GL_ALWAYS    永远通过深度测试
		// GL_NEVER	    永远不通过深度测试
		// GL_LESS	    在片段深度值小于缓冲的深度值时通过测试
		// GL_EQUAL	    在片段深度值等于缓冲区的深度值时通过测试
		// GL_LEQUAL	在片段深度值小于等于缓冲区的深度值时通过测试
		// GL_GREATER	在片段深度值大于缓冲区的深度值时通过测试
		// GL_NOTEQUAL	在片段深度值不等于缓冲区的深度值时通过测试
		// GL_GEQUAL	在片段深度值大于等于缓冲区的深度值时通过测试
		// 
		// 与glEnable(GL_DEPTH_TEST)配套使用，默认是 GL_LESS，即近的覆盖远的
		switch (func)
		{
		case DepthTestFunc::NEVER:    glDepthFunc(GL_NEVER);    break;
		case DepthTestFunc::LESS:     glDepthFunc(GL_LESS);     break;
		case DepthTestFunc::EQUAL:    glDepthFunc(GL_EQUAL);    break;
		case DepthTestFunc::LEQUAL:   glDepthFunc(GL_LEQUAL);   break;
		case DepthTestFunc::GREATER:  glDepthFunc(GL_GREATER);  break;
		case DepthTestFunc::NOTEQUAL: glDepthFunc(GL_NOTEQUAL); break;
		case DepthTestFunc::GEQUAL:   glDepthFunc(GL_GEQUAL);   break;
		case DepthTestFunc::ALWAYS:   glDepthFunc(GL_ALWAYS);   break;
		default:
			break;
		}
	}

	void RendererAPI::SetDepthTestDepthMask(bool enable)
	{
		// true允许写入，false禁止写入
		glDepthMask(enable);
	}

	void RendererAPI::SetBlendEnable(bool enable)
	{
		if (enable)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}

	void RendererAPI::SetBlendFunc(BlendFunc src, BlendFunc dst)
	{
		glBlendFunc(VolcanoToOpenGLBlendFunc(src), VolcanoToOpenGLBlendFunc(dst));
	}

	void RendererAPI::SetBlendEquation(BlendEquation mode)
	{
		switch (mode)
		{
		case BlendEquation::FUNC_ADD:              glBlendEquation(GL_FUNC_ADD);              break;
		case BlendEquation::FUNC_SUBTRACT:         glBlendEquation(GL_FUNC_SUBTRACT);         break;
		case BlendEquation::FUNC_REVERSE_SUBTRACT: glBlendEquation(GL_FUNC_REVERSE_SUBTRACT); break;
		case BlendEquation::MIN:                   glBlendEquation(GL_MIN);                   break;
		case BlendEquation::MAX:                   glBlendEquation(GL_MAX);                   break;
		default:
			break;
		}
	}

	void RendererAPI::SetColorMask(bool r, bool g, bool b, bool a)
	{
		glColorMask(r, g, b, a);
	}

	bool RendererAPI::GetRenderOperationEnabled(RenderOperation enabled)
	{
		switch (enabled)
		{
		case RenderOperation::CULLFACE:
			return (bool)glIsEnabled(GL_CULL_FACE);
		case RenderOperation::STENCIL_TEST:
			return (bool)glIsEnabled(GL_STENCIL_TEST);
		case RenderOperation::DEPTH_TEST:
			return (bool)glIsEnabled(GL_DEPTH_TEST);
		case RenderOperation::BLEND:
			return (bool)glIsEnabled(GL_BLEND);
		default:
			return false;
		}
	}

	int RendererAPI::GetRenderOperationStatus(RenderOperationStatus status)
	{
		int result = 0;
		switch (status)
		{
		case RenderOperationStatus::DEPTH_FUNC:
			glGetIntegerv(GL_DEPTH_FUNC, &result);
			break;
		case RenderOperationStatus::DEPTH_WRITEMASK:
			glGetIntegerv(GL_DEPTH_WRITEMASK, &result);
			break;
		case RenderOperationStatus::STENCIL_FUNC:
			glGetIntegerv(GL_STENCIL_FUNC, &result);
			break;
		case RenderOperationStatus::STENCIL_REF:
			glGetIntegerv(GL_STENCIL_REF, &result);
			break;
		case RenderOperationStatus::STENCIL_VALUE_MASK:
			glGetIntegerv(GL_STENCIL_VALUE_MASK, &result);
			break;
		case RenderOperationStatus::BLEND_SRC_RGB:
			glGetIntegerv(GL_BLEND_SRC_RGB, &result);
			OpenGLBlendFuncToVolcano(result);
			break;
		case RenderOperationStatus::BLEND_DST_RGB:
			glGetIntegerv(GL_BLEND_DST_RGB, &result);
			OpenGLBlendFuncToVolcano(result);
			break;
		default:
			break;
		}
		return result;
	}

}
