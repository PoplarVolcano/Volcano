#pragma once

#include "Texture.h"
#include "glm/glm.hpp"

namespace Volcano {

	enum class FramebufferBufferFormat
	{
		NONE,
		FRONT_LEFT,
		FRONT_RIGHT,
		BACK_LEFT,
		BACK_RIGHT,
		FRONT,
		BACK,
		LEFT,
		RIGHT,
		FRONT_AND_BACK
	};

	// 帧缓冲纹理格式
	enum class FramebufferTextureFormat
	{
		NONE = 0,

		//Color
		RGBA8,
		RGBA16F,
		RED,
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,
		DEPTH_COMPONENT,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	// 帧缓冲纹理规范
	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: textureFormat(format) {
		}
		FramebufferTextureFormat textureFormat = FramebufferTextureFormat::NONE;
		// TODO: filtering/wrap
	};

	// 帧缓冲附件规范
	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification> attachments)
			: Attachments(attachments) {
		}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	// 帧缓冲区规范
	struct FramebufferSpecification
	{
		uint32_t Width, Height;
		glm::vec4 ClearColor = glm::vec4(0.0f);
		FramebufferAttachmentSpecification Attachments;
		uint32_t Samples = 1; // 采样数量：多重采样抗锯齿（MSAA，Multisample Anti-Aliasing）的核心参数
		TextureType ColorType = TextureType::TEXTURE_2D;
		TextureType DepthType = TextureType::TEXTURE_2D;

		bool SwapChainTarget = true;
	};

	class VOL_API Framebuffer
	{
	public:

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual int ReadPixelInt(uint32_t attachmentIndex, int x, int y) = 0;
		virtual float ReadPixelFloat(uint32_t attachmentIndex, int x, int y) = 0;
		virtual void ClearAttachmentInt(uint32_t attachmentIndex, const int value) = 0;
		virtual void ClearAttachmentFloat(uint32_t attachmentIndex, const float value) = 0;

		virtual void BlitStencilFramebuffer(uint32_t srcRendererID, uint32_t dstRendererID, const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1, const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1) = 0;
		virtual void BlitDepthFramebuffer(uint32_t srcRendererID, uint32_t dstRendererID, const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1, const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1) = 0;
		virtual void BlitColorFramebuffer(uint32_t srcRendererID, uint32_t dstRendererID, const std::vector<uint8_t>& indices, const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1, const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1) = 0;

		virtual void BlitColorFramebuffer(uint32_t srcColorAttachmentRendererID, uint32_t dstColorAttachmentRendererID, uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		virtual uint32_t GetDepthAttachmentRendererID() const = 0;
		virtual uint32_t GetRenderbufferObjectRendererID() const = 0;
		virtual const uint32_t GetRendererID() const = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;

		virtual void SetColorAttachment(Ref<Texture> texture, TextureType type = TextureType::TEXTURE_2D, uint32_t index = 0, uint32_t mip = 0) const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}