#pragma once
#include "Volcano/Core/Core.h"

namespace Volcano {

	// 着色器数据类型
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	// 着色器数据类型尺寸
	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:     return 4;
		case ShaderDataType::Float2:    return 4 * 2;
		case ShaderDataType::Float3:    return 4 * 3;
		case ShaderDataType::Float4:    return 4 * 4;
		case ShaderDataType::Mat3:      return 4 * 3 * 3;
		case ShaderDataType::Mat4:      return 4 * 4 * 4;
		case ShaderDataType::Int:       return 4;
		case ShaderDataType::Int2:      return 4 * 2;
		case ShaderDataType::Int3:      return 4 * 3;
		case ShaderDataType::Int4:      return 4 * 4;
		case ShaderDataType::Bool:      return 1;
		}

		VOL_CORE_ASSERT(false, "Unknow ShaderDataType!");
		return 0;
	}

	// 缓冲元素
	struct BufferElement
	{
		ShaderDataType Type;
		std::string Name;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement() = default;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			:Type(type), Name(name), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{}

		// 缓冲元素有几个数，Mat3有3*3=9个数
		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case ShaderDataType::Float:     return 1;
			case ShaderDataType::Float2:    return 2;
			case ShaderDataType::Float3:    return 3;
			case ShaderDataType::Float4:    return 4;
			case ShaderDataType::Mat3:      return 3 * 3;
			case ShaderDataType::Mat4:      return 4 * 4;
			case ShaderDataType::Int:       return 1;
			case ShaderDataType::Int2:      return 2;
			case ShaderDataType::Int3:      return 3;
			case ShaderDataType::Int4:      return 4;
			case ShaderDataType::Bool:      return 1;
			}

			VOL_CORE_ASSERT(false, "Unknow ShaderDataType!");
			return 0;
		}
	};

	// 缓冲布局
	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			:m_Elements(elements)
		{
			CalculateOffsetAndStride();
		}

		// 获取步幅
		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		//计算偏移量和步幅
		void CalculateOffsetAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};


	class VOL_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		static Ref<VertexBuffer> Create(uint32_t size);

		// size -> sizeof(vertices)
		static Ref<VertexBuffer> Create(void* vertices, uint32_t size);
	};

	// Currently Volcano only supports 32-bit index buffers
	class VOL_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		// count -> count * sizeof(uint32_t) 
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};

}