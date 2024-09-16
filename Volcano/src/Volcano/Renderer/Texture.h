#pragma once

#include <string>

#include "Volcano/Core/Base.h"
#include "Volcano/Core/Buffer.h"

namespace Volcano {

	enum class TextureFormat
	{
		None = 0,
		RGB = 1,
		RGBA = 2,
		Float16 = 3
	};

	enum class TextureType
	{
		TEXTURE_2D,
		TEXTURE_2D_MULTISAMPLE,
		TEXTURE_CUBE_MAP
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual void Bind(uint32_t slot = 0) const = 0;
		static void Bind(uint32_t m_RendererID, uint32_t slot);
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
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& path, bool filp = true);
		virtual void SetData(void* data, uint32_t size) = 0;

		virtual const std::string& GetPath() const = 0;
	};

	class TextureCube : public Texture
	{
	public:
		static Ref<TextureCube> Create(TextureFormat format, uint32_t width, uint32_t height);
		static Ref<TextureCube> Create(const std::string& path);
		static Ref<TextureCube> Create(std::vector<std::string> faces);

		virtual const std::string& GetPath() const = 0;
	};
}