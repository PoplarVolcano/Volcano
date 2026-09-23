#pragma once

#include "Volcano/Renderer/Framebuffer.h"

namespace Volcano {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual~OpenGLFramebuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual int ReadPixelInt(uint32_t attachmentIndex, int x, int y) override;
		virtual float ReadPixelFloat(uint32_t attachmentIndex, int x, int y) override;
		virtual void ClearAttachmentInt(uint32_t attachmentIndex, int value) override;
		virtual void ClearAttachmentFloat(uint32_t attachmentIndex, float value) override;

		// 将源帧缓冲的深度缓冲（GL_STENCIL_BUFFER_BIT）拷贝到目标帧缓冲的模板缓冲中，使用最近邻插值
		virtual void BlitStencilFramebuffer(uint32_t srcRendererID, uint32_t dstRendererID, const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1, const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1) override;
		// 将源帧缓冲的深度缓冲（GL_DEPTH_BUFFER_BIT）拷贝到目标帧缓冲的深度缓冲中，使用最近邻插值
		virtual void BlitDepthFramebuffer(uint32_t srcRendererID, uint32_t dstRendererID, const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1, const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1) override;
		// 将源帧缓冲的颜色缓冲（GL_COLOR_BUFFER_BIT）拷贝到目标帧缓冲的颜色缓冲中
		virtual void BlitColorFramebuffer(uint32_t srcRendererID, uint32_t dstRendererID, const std::vector<uint8_t>& indices, const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1, const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1) override;

		virtual void BlitColorFramebuffer(uint32_t srcColorAttachmentRendererID, uint32_t dstColorAttachmentRendererID, uint32_t width, uint32_t height) override;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override
		{
			VOL_CORE_ASSERT(index < m_ColorAttachments.size());
			return m_ColorAttachments[index];
		}
		virtual uint32_t GetDepthAttachmentRendererID() const override { return m_DepthAttachment; }

		virtual uint32_t GetRenderbufferObjectRendererID() const override { return m_RenderbufferObject; }

		virtual const uint32_t GetRendererID() const override { return m_RendererID; }

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

		virtual void SetColorAttachment(Ref<Texture> texture, TextureType type = TextureType::TEXTURE_2D, uint32_t index = 0, uint32_t mip = 0) const override;
	private:
		uint32_t m_RendererID;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::NONE;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;
		uint32_t m_RenderbufferObject = 0;

	};
}