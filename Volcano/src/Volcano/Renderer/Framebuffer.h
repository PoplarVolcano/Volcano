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

	// Ö¡»º³åÎÆÀí¸ñÊ½
	enum class FramebufferTextureFormat
	{
		None = 0,

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

	// Ö¡»º³åÎÆÀí¹æ·¶
	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format) {}
		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		// TODO: filtering/wrap
	};

	// Ö¡»º³å¸½¼þ¹æ·¶
	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification> attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	// Ö¡»º³åÇø¹æ·¶
	struct FramebufferSpecification
	{
		uint32_t Width, Height;
		FramebufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;
		TextureType ColorType = TextureType::TEXTURE_2D;
		TextureType DepthType = TextureType::TEXTURE_2D;

		bool SwapChainTarget = true;
	};

	class Framebuffer
	{
	public:

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;
		virtual float ReadPixelFloat(uint32_t attachmentIndex, int x, int y) = 0;
		virtual void ClearAttachment(uint32_t attachmentIndex, const int value) = 0;

		virtual void SetDrawBuffer(FramebufferBufferFormat format) = 0;
		virtual void SetReadBuffer(FramebufferBufferFormat format) = 0;
		virtual void BlitDepthFramebuffer(uint32_t srcRendererID, uint32_t dstRendererID, const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1, const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1) = 0;
		virtual void BlitColorFramebuffer(uint32_t srcRendererID, uint32_t dstRendererID, const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1, const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1) = 0;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		virtual uint32_t GetDepthAttachmentRendererID() const = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;
		virtual const uint32_t GetRendererID() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}