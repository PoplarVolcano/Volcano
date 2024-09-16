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
				case TextureType::TEXTURE_2D:             return GL_TEXTURE_2D;
				case TextureType::TEXTURE_2D_MULTISAMPLE: return GL_TEXTURE_2D_MULTISAMPLE;
				case TextureType::TEXTURE_CUBE_MAP:       return GL_TEXTURE_CUBE_MAP;
				default:
					return GL_TEXTURE_2D;
			}
		}

		static void CreateTextures(TextureType type, uint32_t* outID, uint32_t count)
		{
			// 纹理附件
			glCreateTextures(TextureTarget(type), count, outID);
		}

		static void AttachColorTexture(uint32_t id, int samples, TextureType type, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
		{
			if (type == TextureType::TEXTURE_2D_MULTISAMPLE)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else if(type == TextureType::TEXTURE_2D)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			// 附加到帧缓冲
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, id, 0);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(type), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, TextureType type, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height, bool storage = true)
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
					glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, NULL);
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
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_FLOAT, NULL);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				}
			}
			// 附加到帧缓冲,glFramebufferTexture2D是旧版本方法
			// glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(type), id, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, id, 0);
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
				case FramebufferTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
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
		for (auto spec : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(spec.TextureFormat))
				m_ColorAttachmentSpecifications.emplace_back(spec);
			else
				m_DepthAttachmentSpecification = spec;
		}
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);

		m_ColorAttachments.clear();
		m_DepthAttachment = 0;
	}

	void OpenGLFramebuffer::Invalidate()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}
		// 创建帧缓冲
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		auto type = m_Specification.ColorType;

		// 纹理缓冲附件
		if (m_ColorAttachmentSpecifications.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
			Utils::CreateTextures(type, m_ColorAttachments.data(), m_ColorAttachments.size());
			for (size_t i = 0; i < m_ColorAttachmentSpecifications.size(); i++)
			{
				Utils::BindTexture(type, m_ColorAttachments[i]);
				switch (m_ColorAttachmentSpecifications[i].TextureFormat)
				{
					case FramebufferTextureFormat::RGBA8:
						Utils::AttachColorTexture(
							m_ColorAttachments[i], 
							m_Specification.Samples, 
							type,
							GL_RGBA8, 
							GL_RGBA,
							m_Specification.Width, 
							m_Specification.Height, i);
						break;

						// 添加整形INTEGER缓冲区附件
					case FramebufferTextureFormat::RED_INTEGER:
						Utils::AttachColorTexture(
							m_ColorAttachments[i],
							m_Specification.Samples,
							type,
							GL_R32I,
							GL_RED_INTEGER,
							m_Specification.Width,
							m_Specification.Height, i);
						break;
				}
			}
		}

		type = m_Specification.DepthType;
		// 深度缓冲附件
		if (m_DepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None)
		{
			Utils::CreateTextures(type, &m_DepthAttachment, 1);
			Utils::BindTexture(type, m_DepthAttachment);
			switch (m_DepthAttachmentSpecification.TextureFormat)
			{
				case FramebufferTextureFormat::DEPTH24STENCIL8:
					// 同时附加一个深度缓冲和一个模板缓冲为一个单独的纹理。纹理的每32位数值就包含了24位的深度信息和8位的模板信息。
					Utils::AttachDepthTexture(
						m_DepthAttachment, 
						m_Specification.Samples, 
						type,
						GL_DEPTH24_STENCIL8, 
						GL_DEPTH_STENCIL_ATTACHMENT, 
						m_Specification.Width, 
						m_Specification.Height);
					break;
				case FramebufferTextureFormat::DEPTH_COMPONENT:
					Utils::AttachDepthTexture(
						m_DepthAttachment,
						m_Specification.Samples,
						type,
						GL_DEPTH_COMPONENT,
						GL_DEPTH_ATTACHMENT,
						m_Specification.Width,
						m_Specification.Height,
						false);
					break;
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			VOL_CORE_ASSERT(m_ColorAttachments.size() <= 4);
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
		VOL_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		// 视图的尺寸，帧缓冲纹理会拉伸到视图尺寸
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);

		int value = -1;
		//glClearTexImage(m_ColorAttachments[1], 0, GL_RED_INTEGER, GL_INT, &value);
	}

	void OpenGLFramebuffer::Unbind()
	{
		// 如果 fboId 参数为 0 ，则代表切换到默认的屏幕上进行绘制。
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Resize后，更新帧缓冲纹理尺寸
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

	int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		VOL_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());
		// 读取第二个缓冲区
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}

	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		VOL_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::VolcanoFBTextureFormatToGL(spec.TextureFormat), GL_INT, &value);
	}

	void OpenGLFramebuffer::SetDrawBuffer(FramebufferBufferFormat format)
	{
		glDrawBuffer(Utils::VolcanoFBBufferFormatToGL(format));
	}

	void OpenGLFramebuffer::SetReadBuffer(FramebufferBufferFormat format)
	{
		glReadBuffer(Utils::VolcanoFBBufferFormatToGL(format));
	}

}