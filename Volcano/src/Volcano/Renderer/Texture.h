#pragma once

#include <string>

#include "Volcano/Core/Core.h"
#include "Volcano/Core/Buffer.h"

#include "glm/glm.hpp"

namespace Volcano {

	// InternalFormat（内部格式）：告诉 GPU “在显存里怎么存”（占用多大显存、用整数还是浮点数、精度多高）。
	/*
	ALPHA	        仅 Alpha 通道
    LUMINANCE	    亮度（灰度）
    LUMINANCE_ALPHA	亮度 + Alpha
    INTENSITY	    强度值

    特殊用途		
    DEPTH_COMPONENT16/24/32F	深度分量格式	
    DEPTH24_STENCIL8	        24位深度 + 8位模板	
    COMPRESSED_*	            压缩纹理格式 (如S3TC, ETC等)
	*/
	enum class TextureInternalFormat
	{
		NONE,

		RED,    // 红色通道（单通道）
		GREEN,  // 绿色通道（单通道）
		BLUE,   // 蓝色通道（单通道）
		RG,	    // 红色 + 绿色（双通道）
		RGB,    // 红绿蓝（三通道）
		RGBA,   // 红绿蓝 + Alpha（四通道）

		// 单通道(R)
		R8,     // 8位无符号归一化整数	ubyte
		R16,    // 16位无符号归一化整数	ushort
		R16F,   // 16位浮点数 (半精度)	half
		R32F,   // 32位浮点数 (单精度)	float
		R8I,    // 8位有符号整数	    byte

		// 双通道(RG)
		RG8,    // 8位无符号归一化整数
		RG16F,  // 16位浮点数 (半精度)
		RG32F,  // 32位浮点数 (单精度)

		// 三通道(RGB)
		RGB4,   // 每通道4位
		RGB8,   // 每通道8位
		RGB16F, // 每通道16位浮点数 (半精度)
		RGB32F, // 每通道32位浮点数 (单精度)

		// 四通道(RGBA)
		RGBA4,   // 每通道4位	
		RGBA8,	 // 每通道8位	
		RGBA16F, // 每通道16位浮点数 (半精度)
		RGBA32F, // 每通道32位浮点数 (单精度)
		RGB10_A2 // RGB各10位, Alpha 2位	
	};

	// DataFormat（数据格式）：告诉 GPU “你刚传来的 CPU 内存数据是什么布局”（RGB顺序还是RGBA、有没有Alpha通道）。
	enum class TextureDataFormat
	{
		NONE,
		RED,			 // 单个颜色分量
		GREEN,			 // 单个颜色分量
		BLUE,	         // 单个颜色分量
		RG,	             // 双通道(红, 绿)
		RGB,	         // 三通道(红, 绿, 蓝)
		BGR,	         // 三通道(蓝, 绿, 红)
		RGBA,	         // 四通道(红, 绿, 蓝, Alpha)
		BGRA,	         // 四通道(蓝, 绿, 红, Alpha)
		RED_INTEGER,	 // 整数格式的单通道
		RG_INTEGER,	     // 整数格式的双通道
		RGB_INTEGER,	 // 整数格式的三通道
		RGBA_INTEGER,	 // 整数格式的四通道
		DEPTH_COMPONENT, // 深度数据
		STENCIL_INDEX,	 // 模板数据
		DEPTH_STENCIL	 // 深度和模板数据
	};

	enum class TextureType
	{
		TEXTURE_2D,
		TEXTURE_2D_MULTISAMPLE,
		TEXTURE_2D_ARRAY,
		TEXTURE_CUBE_MAP,
		TEXTURE_CUBE_MAP_ARRAY,
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

	enum class TextureFilter
	{
		LINEAR,
		LINEAR_MIPMAP_LINEAR
	};

	enum class TextureClassType
	{
		None,    // 无纹理
		Texture2D,
		TextureCube
	};

	struct TextureLibraryKey
	{
		std::string key;
		uint64_t keyHash;
		bool flip = true;
		TextureInternalFormat internalFormat = TextureInternalFormat::NONE;

		bool operator==(const TextureLibraryKey& other) const
		{
			return keyHash == other.keyHash && flip == other.flip && internalFormat == other.internalFormat && key == other.key;
		}
	};


	// map进行查找时调用operator()，如果没有哈希冲突的话就命中，如果有哈希冲突的话调用TextureLibraryKey的operator==
	struct TextureLibraryKeyHash
	{
		size_t operator()(const TextureLibraryKey& k) const
		{
			size_t h = k.keyHash;
			h ^= (size_t)k.flip + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= (size_t)k.internalFormat + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};

	class TextureLibrary;

	class VOL_API Texture
	{
	public:
		static const Scope<TextureLibrary>& GetTextureLibrary();

		virtual ~Texture() = default;

		virtual void Bind(uint32_t slot = 0) const = 0;
		// 静态bind，用于绑定别的纹理
		static void Bind(uint32_t m_RendererID, uint32_t slot);
		static void ClearTextureSlot(uint32_t i);

		// 相对路径
		virtual const std::string& GetRelativePath() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetMipLevelCount() const = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual uint64_t GetHandle() const = 0;

		static uint32_t GetBPP(TextureInternalFormat format);
		static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);

		virtual bool operator==(const Texture& other) const = 0;

		// 是否翻转，默认翻转
		bool GetFlip() { return m_Flip; }
	protected:
		bool m_Flip = true;

		static std::once_flag init_flag;
		static Scope<TextureLibrary> m_TextureLibrary;
	};

	class VOL_API Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(
			uint32_t width,
			uint32_t height,
			TextureInternalFormat internalFormat = TextureInternalFormat::RGBA8,
			TextureDataFormat dataFormat = TextureDataFormat::RGBA,
			TextureWrap wrap = TextureWrap::REPEAT
		);
		static Ref<Texture2D> Create(const std::string& absolutePath, bool flip = true, TextureInternalFormat internalFormat = TextureInternalFormat::NONE);
		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void SetDataFormat(TextureDataFormat format) = 0;
		virtual void SetInternalFormat(TextureInternalFormat format) = 0;
		virtual uint32_t GetDataFormat() = 0;
		virtual uint32_t GetInternalFormat() = 0;
	};

	class VOL_API TextureCube : public Texture
	{
	public:
		static Ref<TextureCube> Create(
			uint32_t width,
			uint32_t height,
			TextureInternalFormat internalFormat = TextureInternalFormat::RGBA8,
			TextureDataFormat dataFormat = TextureDataFormat::RGBA,
			TextureWrap wrap = TextureWrap::CLAMP_TO_EDGE,
			TextureFilter minFilter = TextureFilter::LINEAR,
			TextureFilter maxFilter = TextureFilter::LINEAR
		);
		static Ref<TextureCube> Create(const std::string& path);
		static Ref<TextureCube> Create(std::array<Ref<Texture2D>, 6> faces);
		virtual void SetData(const glm::vec4& color) = 0;
		virtual uint32_t GetInternalFormat() = 0;
		virtual uint32_t GetDataFormat() = 0;
	};

	class VOL_API TextureLibrary
	{
	public:
		TextureLibrary();
		~TextureLibrary() = default;
		
		Ref<Texture2D> Get(const std::string& key, uint64_t& keyHash, bool flip, TextureInternalFormat internalFormat = TextureInternalFormat::NONE);
		
		void CleanupUnused();

		bool Exists(const std::string& key, const uint64_t keyHash, bool flip, TextureInternalFormat internalFormat);
		void Remove(const std::string& key, const uint64_t keyHash, bool flip, TextureInternalFormat internalFormat);
		auto& GetTextures() { return m_Textures; }

	public:
		const uint64_t m_WhiteHash;
		const uint64_t m_BlackHash;

	private:
		// 禁止拷贝
		TextureLibrary(const TextureLibrary&) = delete;
		TextureLibrary& operator=(const TextureLibrary&) = delete;

		void Add(const std::string& key, uint64_t keyHash, bool flip, TextureInternalFormat internalFormat, Ref<Texture2D> texture);

		void Add(const std::string& key, uint64_t keyHash, bool flip, TextureInternalFormat internalFormat);
	
	private:
		std::unordered_map<TextureLibraryKey, Ref<Texture2D>, TextureLibraryKeyHash> m_Textures;
	
	};
}