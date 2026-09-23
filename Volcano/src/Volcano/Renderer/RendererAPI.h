#pragma once

#include "Volcano/Core/Core.h"
#include <glm/glm.hpp>
#include "VertexArray.h"

namespace Volcano {

	// 深度测试函数
	enum class DepthTestFunc
	{
		NEVER,
		LESS,
		EQUAL,
		LEQUAL,
		GREATER,
		NOTEQUAL,
		GEQUAL,
		ALWAYS
	};

	// 面剔除方式
	enum class CullFaceFunc
	{
		FRONT,		   // 剔除背面
		BACK,          // 剔除正面
		FRONT_AND_BACK // 全部剔除
	};

	// 面剔除正面
	enum class CullFaceFrontFace
	{
		CCW, // 逆时针正面
		CW	 // 顺时针正面
	};

	// 多边形模式
	enum class PolygonMode
	{
		POINT, // 点
		LINE,  // 线框
		FILL   // 填充
	};

	enum class StencilFunc
	{
		NEVER,
		LESS,
		EQUAL,
		LEQUAL,
		GREATER,
		NOTEQUAL,
		GEQUAL,
		ALWAYS
	};

	enum class StencilOp
	{
		KEEP,    // 保留
		ZERO,	 // 归零
		REPLACE, // 设为ref
		INCR,	 // +1
		DECR,	 // -1
		INVERT	 // 取反
	};

	enum class BlendFunc
	{
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		ONE,
		ZERO,
		DST_COLOR
	};

	enum class BlendEquation
	{
		FUNC_ADD,              // src * srcFactor + dst * dstFactor
		FUNC_SUBTRACT,         // src * srcFactor - dst * dstFactor
		FUNC_REVERSE_SUBTRACT, // dst * dstFactor - src * srcFactor
		MIN,
		MAX
	};

	enum class RenderOperation
	{
		CULLFACE,
		STENCIL_TEST,
		DEPTH_TEST,
		BLEND
	};

	enum class RenderOperationStatus
	{
		DEPTH_FUNC,
		DEPTH_WRITEMASK,
		STENCIL_FUNC,
		STENCIL_REF,
		STENCIL_VALUE_MASK,
		BLEND_SRC_RGB,
		BLEND_DST_RGB
	};

	enum class RendererAPIType
	{
		None,
		OpenGL
	};

	struct RenderAPICapabilities
	{
		std::string Vendor;
		std::string Renderer;
		std::string Version;

		int MaxSamples = 0;
		float MaxAnisotropy = 0.0f;
		int MaxTextureUnits = 0;
	};

	class RendererAPI
	{
	public:
		static void Init();

		static void SetClearColor(const glm::vec4& color);
		static void Clear();
		static void ClearStencil();


		static void Clear(float r, float g, float b, float a);
		static void ClearStencil(float r, float g, float b, float a);
		static void SetClearColor(float r, float g, float b, float a);
		
		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount);
		static void DrawArrays(const Ref<VertexArray>& vertexArray, uint32_t count);
		static void DrawStripIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount);
		static void DrawIndexInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t amount);
		static void DrawStripIndexInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t amount);

		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount);
		
		static void SetLineWidth(float width);

		static void SetCullFaceEnable(bool enable);
		static void SetCullFaceFunc(CullFaceFunc func);
		static void SetCullFaceFrontFace(CullFaceFrontFace frontFace);

		static void SetPolygonMode(PolygonMode polygonMode);

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		static void SetMultisampleEnable(bool enable);

		static void SetStencilTestEnable(bool enable);
		static void SetStencilFunc(StencilFunc func, int ref, uint32_t mask);
		static void SetStencilOp(StencilOp sfail, StencilOp dpfail, StencilOp dppass);
		static void SetStencilMask(uint32_t mask);

		static void SetDepthTestEnable(bool enable);
		static void SetDepthTestFunc(DepthTestFunc func);
		static void SetDepthTestDepthMask(bool enable);

		static void SetBlendEnable(bool enable);
		static void SetBlendFunc(BlendFunc src, BlendFunc dst);
		static void SetBlendEquation(BlendEquation mode);

		static void SetColorMask(bool r, bool g, bool b, bool a);

		static bool GetRenderOperationEnabled(RenderOperation enabled);
		static int GetRenderOperationStatus(RenderOperationStatus status);
		static RenderAPICapabilities& GetCapabilities()
		{
			static RenderAPICapabilities capabilities;
			return capabilities;
		}

		inline static RendererAPIType Current() { return s_CurrentRendererAPI; }
	private:
		static RendererAPIType s_CurrentRendererAPI;
	};
}