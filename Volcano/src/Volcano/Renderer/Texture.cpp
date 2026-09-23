#include "volpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Volcano/Project/Project.h"
#include "Volcano/Platform/OpenGL/OpenGLTexture.h"

namespace Volcano {

	std::once_flag Texture::init_flag;
	Scope<TextureLibrary> Texture::m_TextureLibrary;

	const Scope<TextureLibrary>& Texture::GetTextureLibrary()
	{
		std::call_once(init_flag, []() { m_TextureLibrary.reset(new TextureLibrary()); });
		return m_TextureLibrary;
	}

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, TextureInternalFormat internalFormat, TextureDataFormat dataFormat, TextureWrap wrap)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    VOL_CORE_ASSERT(false, "Texture2D：API为None不支持"); return nullptr;
		case RendererAPIType::OpenGL:  return  CreateRef<OpenGLTexture2D>(width, height, internalFormat, dataFormat, wrap);
		}
		VOL_CORE_ASSERT(false, "Buffer：未知API");
		return nullptr;
	}

	// path: 纹理绝对路径
	Ref<Texture2D> Texture2D::Create(const std::string& absolutePath, bool flip, TextureInternalFormat internalFormat)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    VOL_CORE_ASSERT(false, "Texture2D：API为None不支持"); return nullptr;
		case RendererAPIType::OpenGL:  return  std::make_shared<OpenGLTexture2D>(absolutePath, flip, internalFormat);
		}
		VOL_CORE_ASSERT(false, "Buffer：未知API");
		return nullptr;
	}

	uint32_t Texture::GetBPP(TextureInternalFormat format)
	{
		switch (format)
		{
		case TextureInternalFormat::RGB:    return 3;
		case TextureInternalFormat::RGBA:   return 4;
		}
		return 0;
	}

	uint32_t Texture::CalculateMipMapCount(uint32_t width, uint32_t height)
	{
		uint32_t levels = 1;
		while ((width | height) >> levels)
			levels++;

		return levels;
	}

	// ==================================TextureCube=================================================

	Ref<TextureCube> TextureCube::Create(
		uint32_t width,
		uint32_t height,
		TextureInternalFormat internalFormat,
		TextureDataFormat dataFormat,
		TextureWrap wrap,
		TextureFilter minFilter,
		TextureFilter maxFilter
	)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None: return nullptr;
		case RendererAPIType::OpenGL: return CreateRef<OpenGLTextureCube>(width, height, internalFormat, dataFormat, wrap, minFilter, maxFilter);
		}
		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(const std::string& path)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None: return nullptr;
		case RendererAPIType::OpenGL: return CreateRef<OpenGLTextureCube>(path);
		}
		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(std::array<Ref<Texture2D>, 6> faces)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None: return nullptr;
		case RendererAPIType::OpenGL: return CreateRef<OpenGLTextureCube>(faces);
		}
		return nullptr;
	}

    // ==================================TextureLibrary=================================================
	
    TextureLibrary::TextureLibrary()
		: m_WhiteHash(std::hash<std::string>{}("White")), m_BlackHash(std::hash<std::string>{}("Black"))
    {
        Ref<Texture2D> whiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
        whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
        Add("White", m_WhiteHash, true, TextureInternalFormat::NONE, whiteTexture);
		Add("White", m_WhiteHash, false, TextureInternalFormat::NONE, whiteTexture);

        Ref<Texture2D> blackTexture = Texture2D::Create(1, 1);
		uint32_t blackTextureData = 0x00000000;
        blackTexture->SetData(&blackTextureData, sizeof(uint32_t));
        Add("Black", m_BlackHash, true, TextureInternalFormat::NONE, blackTexture);
		Add("Black", m_BlackHash, false, TextureInternalFormat::NONE, blackTexture);
    }

    Ref<Texture2D> TextureLibrary::Get(const std::string& key, uint64_t& keyHash, bool flip, TextureInternalFormat internalFormat)
    {
		if (keyHash == 0)
		{
			keyHash = std::hash<std::string>{}(key);
		}

        auto it = m_Textures.find(TextureLibraryKey(key, keyHash, flip, internalFormat));

		if (it != m_Textures.end())
		{
			return it->second;
		}
		else
        {
			// 如果未命中，可能目标不存在或keyHash脏数据
			// 重置keyHash后再次搜索，未命中则是目标不存在，添加目标，返回
			keyHash = std::hash<std::string>{}(key);
			it = m_Textures.find(TextureLibraryKey(key, keyHash, flip, internalFormat));
			if (it != m_Textures.end())
			{
				return it->second;
			}

            Add(key, keyHash, flip, internalFormat);
            it = m_Textures.find(TextureLibraryKey(key, keyHash, flip, internalFormat));
			if (it != m_Textures.end())
			{
				return it->second;
			}
			else
            {
                VOL_CORE_ASSERT("ModelLibrary::Get：未找到Texture");
                return nullptr;
            }
        }
    }

    void TextureLibrary::Add(const std::string& key, uint64_t keyHash, bool flip, TextureInternalFormat internalFormat, Ref<Texture2D> texture)
    {
        VOL_CORE_ASSERT(texture != nullptr, "TextureLibrary::Add: Texture should not be a null pointer");
        
		m_Textures[TextureLibraryKey(key, keyHash, flip, internalFormat)] = texture;
    }

    void TextureLibrary::Add(const std::string& key, uint64_t keyHash, bool flip, TextureInternalFormat internalFormat)
    {
		auto textureAbsolutePath = Project::GetAssetsAbsolutePath() / key;
        Ref<Texture2D> texture = Texture2D::Create(textureAbsolutePath.string(), flip, internalFormat);
        Add(key, keyHash, flip, internalFormat, texture);
    }

    void TextureLibrary::CleanupUnused()
    {
        size_t before = m_Textures.size();
        size_t removed = 0;

        // 遍历所有纹理
        for (auto it = m_Textures.begin(); it != m_Textures.end(); ) {
            // use_count() == 1 表示只有 Library 持有该纹理
            // 注意：use_count() 包括当前 shared_ptr 和所有拷贝
			if (it->second.use_count() == 1 && it->first.keyHash != m_WhiteHash && it->first.keyHash != m_BlackHash)
			{
				VOL_CORE_INFO("移除未使用的纹理: {}", it->first.key);
                it = m_Textures.erase(it);
                removed++;
            }
            else {
                ++it;
            }
        }

        size_t after = m_Textures.size();
        VOL_CORE_INFO("纹理清理完成: 移除 {} 个，剩余 {} 个 (之前 {})", removed, after, before);
    }

    bool TextureLibrary::Exists(const std::string& key, const uint64_t keyHash, bool flip, TextureInternalFormat internalFormat)
    {
        return m_Textures.find(TextureLibraryKey(key, keyHash, flip, internalFormat)) != m_Textures.end();
    }

    void TextureLibrary::Remove(const std::string& key, const uint64_t keyHash, bool flip, TextureInternalFormat internalFormat)
    {
        m_Textures.erase(TextureLibraryKey(key, keyHash, flip, internalFormat));
    }

}