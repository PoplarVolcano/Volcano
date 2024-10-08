#pragma once

#include <string>

#include "Volcano/Core/Base.h"
#include "Volcano/Core/Buffer.h"

namespace Volcano {

	enum class TextureFormat
	{
		None,
		RG,
		RGB,
		RGBA,
		RGBA8,
		RG16F,
		RGB16F,
		RGBA16F
	};

	enum class TextureType
	{
		TEXTURE_2D,
		TEXTURE_2D_MULTISAMPLE,
		TEXTURE_CUBE_MAP,
		TEXTURE_CUBE_MAP_POSITIVE_X,
		TEXTURE_CUBE_MAP_NEGATIVE_X,
		TEXTURE_CUBE_MAP_POSITIVE_Y,
		TEXTURE_CUBE_MAP_NEGATIVE_Y,
		TEXTURE_CUBE_MAP_POSITIVE_Z,
		TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	enum class TextureWrap
	{
		REPEAT,
		CLAMP_TO_EDGE
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual void Bind(uint32_t slot = 0) const = 0;
		static void Bind(uint32_t m_RendererID, uint32_t slot);
		static void ClearTextureSlot(uint32_t i);

		virtual const std::string& GetPath() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetMipLevelCount() const = 0;

		virtual uint32_t GetRendererID() const = 0;

		static uint32_t GetBPP(TextureFormat format);
		static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, TextureFormat internalFormat = TextureFormat::RGBA8, TextureFormat dataFormat = TextureFormat::RGBA, TextureWrap wrap = TextureWrap::REPEAT);
		static Ref<Texture2D> Create(const std::string& path, bool filp = true, TextureFormat internalFormat = TextureFormat::None);
		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void SetDataFormat(TextureFormat format) = 0;
		virtual void SetInternalFormat(TextureFormat format) = 0;

	};

	class TextureCube : public Texture
	{
	public:
		static Ref<TextureCube> Create(TextureFormat format, uint32_t width, uint32_t height);
		static Ref<TextureCube> Create(const std::string& path);
		static Ref<TextureCube> Create(std::vector<std::string> faces);
	};
}