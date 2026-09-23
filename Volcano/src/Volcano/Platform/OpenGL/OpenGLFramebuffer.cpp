#include "volpch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Volcano {

	static const uint32_t MaxFramebufferSize = 8192;

	namespace Utils {

		static GLenum TextureTarget(TextureType type)
		{
			switch (type)
			{
			case TextureType::TEXTURE_2D:                  return GL_TEXTURE_2D;
			case TextureType::TEXTURE_2D_MULTISAMPLE:      return GL_TEXTURE_2D_MULTISAMPLE;
			case TextureType::TEXTURE_2D_ARRAY:            return GL_TEXTURE_2D_ARRAY;
			case TextureType::TEXTURE_CUBE_MAP:            return GL_TEXTURE_CUBE_MAP;
			case TextureType::TEXTURE_CUBE_MAP_ARRAY:      return GL_TEXTURE_CUBE_MAP_ARRAY;
			case TextureType::TEXTURE_CUBE_MAP_POSITIVE_X: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			case TextureType::TEXTURE_CUBE_MAP_NEGATIVE_X: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			case TextureType::TEXTURE_CUBE_MAP_POSITIVE_Y: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			case TextureType::TEXTURE_CUBE_MAP_NEGATIVE_Y: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			case TextureType::TEXTURE_CUBE_MAP_POSITIVE_Z: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			case TextureType::TEXTURE_CUBE_MAP_NEGATIVE_Z: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			default:
				return GL_TEXTURE_2D;
			}
		}

		static void CreateTextures(TextureType type, uint32_t count, uint32_t* outID)
		{
			// 纹理附件
			glCreateTextures(TextureTarget(type), count, outID);
		}

		// colorAttachment：颜色附件ColorAttachment
		// samples：采样数量：多重采样抗锯齿（MSAA，Multisample Anti-Aliasing）的核心参数
		// internalFormat：指定OpenGL是如何管理纹理单元中数据格式的,告诉驱动它里面的数据是怎么组织的
		// format：指定data所指向的数据的格式
		// index：颜色附件的索引
		static void AttachColorTexture(uint32_t colorAttachment, int samples, TextureType type, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
		{
			if (type == TextureType::TEXTURE_2D_MULTISAMPLE)
			{
				// 创建多采样（Multisample）纹理，实现多重采样抗锯齿（MSAA，Multisample Anti-Aliasing）
				// 它告诉GPU：“我需要一块能存储多个样本点（Samples）的纹理，用于抗锯齿。” 
				// 与普通的glTexImage2D不同，这个函数创建的是一个特殊的2D纹理，它的每个像素都包含了多个子样本（Sub-samples）。
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else if (type == TextureType::TEXTURE_2D)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else if (type == TextureType::TEXTURE_CUBE_MAP)
			{
				for (unsigned int i = 0; i < 6; ++i)
				{
					// 注意，我们使用16位浮点数来存储每个面
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
				}
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			}
			// 附加到帧缓冲
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, colorAttachment, 0);
		}

		static void AttachDepthTexture(uint32_t depthAttachment, int samples, TextureType type, GLenum format, GLenum depthAttachmentType, uint32_t width, uint32_t height, bool storage = true)
		{
			if (type == TextureType::TEXTURE_2D_MULTISAMPLE)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else if (type == TextureType::TEXTURE_2D)
			{
				if (storage)
				{
					glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
				else
				{
					glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
					float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
					glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
				}

			}
			else if (type == TextureType::TEXTURE_CUBE_MAP)
			{
				if (storage)
				{
				}
				else
				{
					for (uint32_t i = 0; i < 6; ++i)
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				}
			}
			// 附加到帧缓冲,glFramebufferTexture2D是旧版本方法
			// glFramebufferTexture2D(GL_FRAMEBUFFER, depthAttachmentType, TextureTarget(type), depthAttachment, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, depthAttachmentType, depthAttachment, 0);
		}
		static void BindTexture(TextureType type, uint32_t id)
		{
			glBindTexture(TextureTarget(type), id);
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
			case FramebufferTextureFormat::DEPTH_COMPONENT: return true;
			}
			return false;
		}

		static GLenum VolcanoFBTextureFormatToGL(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::RGBA8:       return GL_RGBA8;
			case FramebufferTextureFormat::RGBA16F:     return GL_RGBA16F;
			case FramebufferTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
			case FramebufferTextureFormat::RED:         return GL_RED;
			}
			VOL_CORE_ASSERT(false);
			return 0;
		}

		static GLenum VolcanoFBBufferFormatToGL(FramebufferBufferFormat format)
		{
			switch (format)
			{
			case FramebufferBufferFormat::NONE:           return GL_NONE;
			case FramebufferBufferFormat::FRONT_LEFT:     return GL_FRONT_LEFT;
			case FramebufferBufferFormat::FRONT_RIGHT:    return GL_FRONT_RIGHT;
			case FramebufferBufferFormat::BACK_LEFT:      return GL_BACK_LEFT;
			case FramebufferBufferFormat::BACK_RIGHT:     return GL_BACK_RIGHT;
			case FramebufferBufferFormat::FRONT:          return GL_FRONT;
			case FramebufferBufferFormat::BACK:           return GL_BACK;
			case FramebufferBufferFormat::LEFT:           return GL_LEFT;
			case FramebufferBufferFormat::RIGHT:          return GL_RIGHT;
			case FramebufferBufferFormat::FRONT_AND_BACK: return GL_FRONT_AND_BACK;
			}
			VOL_CORE_ASSERT(false);
			return 0;
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		// 颜色附件数量上限GL_MAX_COLOR_ATTACHMENTS，深度附件固定一个
		// 预处理，将spec中的颜色附件参数和深度附件参数保存到成员变量
		for (auto spec : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(spec.textureFormat))
				m_ColorAttachmentSpecifications.push_back(spec);
			else
				m_DepthAttachmentSpecification = spec;
		}
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);

		m_ColorAttachments.clear();
		m_DepthAttachment = 0;
	}

	void OpenGLFramebuffer::Invalidate()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		auto type = m_Specification.ColorType;
		if (m_ColorAttachmentSpecifications.size() != 0)
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
			// glCreateTextures(TextureTarget(type), count, outID);
			Utils::CreateTextures(type, (uint32_t)m_ColorAttachments.size(), m_ColorAttachments.data());
			GLenum internalFormat = 0, format = 0;
			for (int i = 0; i < m_ColorAttachmentSpecifications.size(); i++)
			{
				// glBindTexture(TextureTarget(type), id);
				Utils::BindTexture(type, m_ColorAttachments[i]);
				switch (m_ColorAttachmentSpecifications[i].textureFormat)
				{
				case FramebufferTextureFormat::RGBA8:
					internalFormat = GL_RGBA8;
					format = GL_RGBA;
					break;
				case FramebufferTextureFormat::RGBA16F: // 用于HDR渲染，16位每颜色分量的浮点帧缓冲
					internalFormat = GL_RGBA16F;
					format = GL_RGBA;
					break;
				case FramebufferTextureFormat::RED_INTEGER:// 整形INTEGER缓冲区附件
					internalFormat = GL_R32I;
					format = GL_RED_INTEGER;
					break;
				case FramebufferTextureFormat::RED:
					internalFormat = GL_RED;
					format = GL_RGB;
					break;
				default:
					VOL_CORE_ASSERT(false, "OpenGLFramebuffer->ColorAttachment->FramebufferTextureFormat.");
					break;
				}
				Utils::AttachColorTexture(
					m_ColorAttachments[i],
					m_Specification.Samples,
					type,
					internalFormat,
					format,
					m_Specification.Width,
					m_Specification.Height,
					i);
			}
		}

		type = m_Specification.DepthType;
		if (m_DepthAttachmentSpecification.textureFormat != FramebufferTextureFormat::NONE)
		{
			GLenum format = 0, depthAttachmentType = 0;
			bool storage = true;
			Utils::CreateTextures(type, 1, &m_DepthAttachment);
			Utils::BindTexture(type, m_DepthAttachment);
			switch (m_DepthAttachmentSpecification.textureFormat)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				// 同时附加一个深度缓冲和一个模板缓冲为一个单独的纹理。
				// 纹理每32位数值包含24位的深度信息和8位的模板信息。
				format = GL_DEPTH24_STENCIL8;
				depthAttachmentType = GL_DEPTH_STENCIL_ATTACHMENT;
				storage = true;
				break;
			case FramebufferTextureFormat::DEPTH_COMPONENT:
				format = GL_DEPTH_COMPONENT;
				depthAttachmentType = GL_DEPTH_ATTACHMENT;
				storage = false;
				break;
			default:
				VOL_CORE_ASSERT(false, "OpenGLFramebuffer->DepthAttachment->FramebufferTextureFormat.");
				break;
			}
			Utils::AttachDepthTexture(
				m_DepthAttachment,
				m_Specification.Samples,
				type,
				format,
				depthAttachmentType,
				m_Specification.Width,
				m_Specification.Height,
				storage);
		}

		if (m_ColorAttachments.size() > 1)
		{
			// 多个颜色附件时，设置缓冲区，限制最多8个颜色附件
			VOL_CORE_ASSERT(m_ColorAttachments.size() < 9);
			GLenum buffers[8] = {
				GL_COLOR_ATTACHMENT0,
				GL_COLOR_ATTACHMENT1,
				GL_COLOR_ATTACHMENT2,
				GL_COLOR_ATTACHMENT3,
				GL_COLOR_ATTACHMENT4,
				GL_COLOR_ATTACHMENT5,
				GL_COLOR_ATTACHMENT6,
				GL_COLOR_ATTACHMENT7
			};
			glDrawBuffers((GLsizei)m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty() && m_DepthAttachmentSpecification.textureFormat != FramebufferTextureFormat::NONE)
		{
			// 没有颜色附件，只有深度附件
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		// 如果颜色附件和深度附件都为空，设置渲染缓冲对象GL_RENDERBUFFER
		if (m_ColorAttachments.empty() && m_DepthAttachmentSpecification.textureFormat == FramebufferTextureFormat::NONE)
		{
			glGenRenderbuffers(1, &m_RenderbufferObject);
			glBindRenderbuffer(GL_RENDERBUFFER, m_RenderbufferObject);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_Specification.Width, m_Specification.Height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RenderbufferObject);
		}

		VOL_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		// 视图的尺寸，帧缓冲纹理会拉伸到视图尺寸
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);

	}

	void OpenGLFramebuffer::Unbind()
	{
		// fboId == 0代表切换到默认的屏幕上进行绘制。
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > MaxFramebufferSize || height > MaxFramebufferSize)
		{
			VOL_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	int OpenGLFramebuffer::ReadPixelInt(uint32_t attachmentIndex, int x, int y)
	{
		VOL_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());
		Bind();
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		Unbind();
		return pixelData;
	}

	float OpenGLFramebuffer::ReadPixelFloat(uint32_t attachmentIndex, int x, int y)
	{
		VOL_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());
		// 读取第attachmentIndex个缓冲区
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		float pixelData;
		glReadPixels(x, y, 1, 1, GL_RED, GL_FLOAT, &pixelData);
		return pixelData;
	}

	void OpenGLFramebuffer::ClearAttachmentInt(uint32_t attachmentIndex, int value)
	{
		VOL_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::VolcanoFBTextureFormatToGL(spec.textureFormat), GL_INT, &value);
	}

	void OpenGLFramebuffer::ClearAttachmentFloat(uint32_t attachmentIndex, float value)
	{
		VOL_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::VolcanoFBTextureFormatToGL(spec.textureFormat), GL_FLOAT, &value);
	}

	void OpenGLFramebuffer::BlitStencilFramebuffer(
		uint32_t srcRendererID, uint32_t dstRendererID,
		const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1,
		const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, srcRendererID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstRendererID);
		glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	}

	void OpenGLFramebuffer::BlitDepthFramebuffer(
		uint32_t srcRendererID, uint32_t dstRendererID,
		const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1,
		const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, srcRendererID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstRendererID);
		glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}

	void OpenGLFramebuffer::BlitColorFramebuffer(
		uint32_t srcRendererID, uint32_t dstRendererID,
		const std::vector<uint8_t>& indices,
		const uint32_t srcX0, const uint32_t srcY0, const uint32_t srcX1, const uint32_t srcY1,
		const uint32_t dstX0, const uint32_t dstY0, const uint32_t dstX1, const uint32_t dstY1)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, srcRendererID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstRendererID);
		for (uint8_t i = 0; i != indices.size(); i++)
		{
			glReadBuffer(GL_COLOR_ATTACHMENT0 + indices[i]);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + indices[i]);
			glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
	}

	void OpenGLFramebuffer::BlitColorFramebuffer(uint32_t srcColorAttachmentRendererID, uint32_t dstColorAttachmentRendererID, uint32_t width, uint32_t height)
	{
		if (!srcColorAttachmentRendererID || !dstColorAttachmentRendererID) {
			VOL_CORE_ERROR("Invalid attachment textures for BlitColorAttachment");
			return;
		}

		glCopyImageSubData(
			srcColorAttachmentRendererID, GL_TEXTURE_2D, 0, 0, 0, 0,          // 源：纹理ID，目标类型，mip级别，x,y,z
			dstColorAttachmentRendererID, GL_TEXTURE_2D, 0, 0, 0, 0,          // 目标：纹理ID，目标类型，mip级别，x,y,z
			width, height, 1                                                  // 拷贝尺寸（深度=1）
		);
	}

	void OpenGLFramebuffer::SetColorAttachment(Ref<Texture> texture, TextureType type, uint32_t index, uint32_t mip) const
	{
		if (texture)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, Utils::TextureTarget(type), texture->GetRendererID(), mip);
		}
	}
}