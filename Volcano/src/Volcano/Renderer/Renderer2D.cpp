#include "volpch.h"
#include "Volcano/Renderer/Renderer2D.h"

#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Shader.h"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/UniformBuffer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Volcano {

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoords;
		float TextureIndex;
		float TilingFactor;

		// Editor-only
		int EntityID;
	};

	struct CircleVertex
	{
		glm::vec3 WorldPosition;
		glm::vec3 LocalPosition;
		glm::vec4 Color;
		float Thickness;
		float Fade;

		// Editor-only
		int EntityID;
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;

		// Editor-only
		int EntityID;
	};

	struct Renderer2DData
	{
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;// RenderCaps

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;
		Ref<Texture2D> WhiteTexture;

		Ref<VertexArray> CircleVertexArray;
		Ref<VertexBuffer> CircleVertexBuffer;
		Ref<Shader> CircleShader;

		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		Ref<Shader> LineShader;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		uint32_t CircleIndexCount = 0;
		CircleVertex* CircleVertexBufferBase = nullptr;
		CircleVertex* CircleVertexBufferPtr = nullptr;

		uint32_t LineVertexCount = 0;  // Line只需要提供顶点数量
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;
		float LineWidth = 2.0f;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;// 0 = white texture

		glm::vec4 QuadVertexPosition[4];

		Renderer2D::Statistics Stats;
	};
	static Renderer2DData s_Renderer2DData;


	void Renderer2D::Init()
	{
		// Quad
		s_Renderer2DData.QuadVertexArray = VertexArray::Create();
		s_Renderer2DData.QuadVertexBuffer = VertexBuffer::Create(s_Renderer2DData.MaxVertices * sizeof(QuadVertex));
		s_Renderer2DData.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float2, "a_TexCoords"     },
			{ ShaderDataType::Float,  "a_TextureIndex" },
			{ ShaderDataType::Float,  "a_TilingFactor" },
			{ ShaderDataType::Int,    "a_EntityID"     }
			});
		s_Renderer2DData.QuadVertexArray->AddVertexBuffer(s_Renderer2DData.QuadVertexBuffer);
		s_Renderer2DData.QuadVertexBufferBase = new QuadVertex[s_Renderer2DData.MaxVertices];
		uint32_t* quadIndices = new uint32_t[s_Renderer2DData.MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Renderer2DData.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, s_Renderer2DData.MaxIndices);;
		s_Renderer2DData.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
		delete[] quadIndices;	// cpu上传到gpu上了可以删除cpu的索引数据块了

		// Circles
		s_Renderer2DData.CircleVertexArray = VertexArray::Create();
		s_Renderer2DData.CircleVertexBuffer = VertexBuffer::Create(s_Renderer2DData.MaxVertices * sizeof(CircleVertex));
		s_Renderer2DData.CircleVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_WorldPosition" },
			{ ShaderDataType::Float3, "a_LocalPosition" },
			{ ShaderDataType::Float4, "a_Color"         },
			{ ShaderDataType::Float,  "a_Thickness"     },
			{ ShaderDataType::Float,  "a_Fade"          },
			{ ShaderDataType::Int,    "a_EntityID"      }
			});
		s_Renderer2DData.CircleVertexArray->AddVertexBuffer(s_Renderer2DData.CircleVertexBuffer);
		s_Renderer2DData.CircleVertexArray->SetIndexBuffer(quadIndexBuffer); // Use quad IB
		s_Renderer2DData.CircleVertexBufferBase = new CircleVertex[s_Renderer2DData.MaxVertices];

		// Lines
		s_Renderer2DData.LineVertexArray = VertexArray::Create();
		s_Renderer2DData.LineVertexBuffer = VertexBuffer::Create(s_Renderer2DData.MaxVertices * sizeof(LineVertex));
		s_Renderer2DData.LineVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"    },
			{ ShaderDataType::Int,    "a_EntityID" }
			});
		s_Renderer2DData.LineVertexArray->AddVertexBuffer(s_Renderer2DData.LineVertexBuffer);
		s_Renderer2DData.LineVertexBufferBase = new LineVertex[s_Renderer2DData.MaxVertices];

		s_Renderer2DData.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Renderer2DData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[s_Renderer2DData.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Renderer2DData.MaxTextureSlots; i++)
			samplers[i] = i;

		Renderer::GetShaderLibrary()->Load("assets/shaders/Renderer2D_Quad.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/Renderer2D_Circle.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/Renderer2D_Line.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/window.glsl");
		s_Renderer2DData.QuadShader = Renderer::GetShaderLibrary()->Get("Renderer2D_Quad");
		s_Renderer2DData.CircleShader = Renderer::GetShaderLibrary()->Get("Renderer2D_Circle");
		s_Renderer2DData.LineShader = Renderer::GetShaderLibrary()->Get("Renderer2D_Line");

		s_Renderer2DData.TextureSlots[0] = s_Renderer2DData.WhiteTexture;

		s_Renderer2DData.QuadVertexPosition[0] = { -0.5, -0.5, 0.0f, 1.0f };
		s_Renderer2DData.QuadVertexPosition[1] = { 0.5, -0.5, 0.0f, 1.0f };
		s_Renderer2DData.QuadVertexPosition[2] = { 0.5,  0.5, 0.0f, 1.0f };
		s_Renderer2DData.QuadVertexPosition[3] = { -0.5,  0.5, 0.0f, 1.0f };

		s_CameraUniformBuffer = UniformBuffer::Create(sizeof(CameraData), 0);

	}

	void Renderer2D::Shutdown()
	{
		delete[] s_Renderer2DData.QuadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		s_CameraBuffer.viewProjection = camera.GetViewProjection();
		s_CameraUniformBuffer->SetData(&s_CameraBuffer, sizeof(CameraData));

		StartBatch();
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		s_CameraBuffer.viewProjection = camera.GetProjection() * glm::inverse(transform);
		s_CameraUniformBuffer->SetData(&s_CameraBuffer, sizeof(CameraData));

		StartBatch();
	}


	void Renderer2D::EndScene()
	{
		Flush();
	}

	void Renderer2D::Flush()
	{
		if (s_Renderer2DData.QuadIndexCount)
		{
			// 不加uint8_t转化会得到元素数量, 加uint8_t返回以char为单位占用多少元素
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Renderer2DData.QuadVertexBufferPtr - (uint8_t*)s_Renderer2DData.QuadVertexBufferBase);
			s_Renderer2DData.QuadVertexBuffer->SetData(s_Renderer2DData.QuadVertexBufferBase, dataSize);

			// Bind textures
			for (uint32_t i = 0; i < s_Renderer2DData.TextureSlotIndex; i++)
				s_Renderer2DData.TextureSlots[i]->Bind(i);

			s_Renderer2DData.QuadShader->Bind();
			Renderer::DrawIndexed(s_Renderer2DData.QuadVertexArray, s_Renderer2DData.QuadIndexCount);
			s_Renderer2DData.Stats.DrawCalls++;
		}

		if (s_Renderer2DData.CircleIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Renderer2DData.CircleVertexBufferPtr - (uint8_t*)s_Renderer2DData.CircleVertexBufferBase);
			s_Renderer2DData.CircleVertexBuffer->SetData(s_Renderer2DData.CircleVertexBufferBase, dataSize);

			s_Renderer2DData.CircleShader->Bind();
			Renderer::DrawIndexed(s_Renderer2DData.CircleVertexArray, s_Renderer2DData.CircleIndexCount);
			s_Renderer2DData.Stats.DrawCalls++;
		}

		if (s_Renderer2DData.LineVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Renderer2DData.LineVertexBufferPtr - (uint8_t*)s_Renderer2DData.LineVertexBufferBase);
			s_Renderer2DData.LineVertexBuffer->SetData(s_Renderer2DData.LineVertexBufferBase, dataSize);

			s_Renderer2DData.LineShader->Bind();
			Renderer::SetLineWidth(s_Renderer2DData.LineWidth);
			Renderer::DrawLines(s_Renderer2DData.LineVertexArray, s_Renderer2DData.LineVertexCount);
			s_Renderer2DData.Stats.DrawCalls++;
		}
	}

	void Renderer2D::StartBatch()
	{

		s_Renderer2DData.QuadIndexCount = 0;
		s_Renderer2DData.QuadVertexBufferPtr = s_Renderer2DData.QuadVertexBufferBase;

		s_Renderer2DData.CircleIndexCount = 0;
		s_Renderer2DData.CircleVertexBufferPtr = s_Renderer2DData.CircleVertexBufferBase;

		s_Renderer2DData.LineVertexCount = 0;
		s_Renderer2DData.LineVertexBufferPtr = s_Renderer2DData.LineVertexBufferBase;

		s_Renderer2DData.TextureSlotIndex = 1;
	}

	void Renderer2D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x,position.y,0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		DrawQuad(transform, color);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& color)
	{
		DrawQuad({ position.x,position.y,0.0f }, size, texture, tilingFactor, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		DrawQuad(transform, texture, tilingFactor, color);
	}

	/*
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4& color)
	{
		DrawQuad({ position.x,position.y,0.0f }, size, subTexture, tilingFactor, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4& color)
	{

		const glm::vec2* textureCoords = subTexture->GetTexCoords();
		const Ref<Texture2D> texture = subTexture->GetTexture();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		if (s_Renderer2DData.QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;

		// 遍历纹理，是否已注入
		for (uint32_t i = 1; i < s_Renderer2DData.TextureSlotIndex; i++)
		{
			if (*s_Renderer2DData.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		//未注入，将texture注入，索引为TextureSlotIndex（从1开始计数）
		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Renderer2DData.TextureSlotIndex;
			s_Renderer2DData.TextureSlots[s_Renderer2DData.TextureSlotIndex] = texture;
			s_Renderer2DData.TextureSlotIndex++;
		}

		for (uint32_t i = 0; i < 4; i++) {
			s_Renderer2DData.QuadVertexBufferPtr->Position = transform * s_Renderer2DData.QuadVertexPosition[i];
			s_Renderer2DData.QuadVertexBufferPtr->Color = color;
			s_Renderer2DData.QuadVertexBufferPtr->TexCoords = textureCoords[i];
			s_Renderer2DData.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_Renderer2DData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Renderer2DData.QuadVertexBufferPtr++;
		}

		s_Renderer2DData.QuadIndexCount += 6;

		s_Renderer2DData.Stats.QuadCount++;
	}
	*/
	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		if (s_Renderer2DData.QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		const float tilingFactor = 1.0f;

		// 逆时针注入顶点数据
		// 设置顶点的地址指向注入的地址
		for (uint32_t i = 0; i < 4; i++) {
			s_Renderer2DData.QuadVertexBufferPtr->Position = transform * s_Renderer2DData.QuadVertexPosition[i];
			s_Renderer2DData.QuadVertexBufferPtr->Color = color;
			s_Renderer2DData.QuadVertexBufferPtr->TexCoords = textureCoords[i];
			s_Renderer2DData.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_Renderer2DData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Renderer2DData.QuadVertexBufferPtr->EntityID = entityID;
			s_Renderer2DData.QuadVertexBufferPtr++;
		}

		s_Renderer2DData.QuadIndexCount += 6;

		s_Renderer2DData.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& color, int entityID)
	{
		if (s_Renderer2DData.QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		// 遍历纹理，是否已注入
		for (uint32_t i = 1; i < s_Renderer2DData.TextureSlotIndex; i++)
		{
			if (*s_Renderer2DData.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		//未注入，将texture注入，索引为TextureSlotIndex（从1开始计数）
		if (textureIndex == 0.0f)
		{
			if (s_Renderer2DData.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
				NextBatch();
			textureIndex = (float)s_Renderer2DData.TextureSlotIndex;
			s_Renderer2DData.TextureSlots[s_Renderer2DData.TextureSlotIndex] = texture;
			s_Renderer2DData.TextureSlotIndex++;
		}

		for (uint32_t i = 0; i < 4; i++) {
			s_Renderer2DData.QuadVertexBufferPtr->Position = transform * s_Renderer2DData.QuadVertexPosition[i];
			s_Renderer2DData.QuadVertexBufferPtr->Color = color;
			s_Renderer2DData.QuadVertexBufferPtr->TexCoords = textureCoords[i];
			s_Renderer2DData.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_Renderer2DData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Renderer2DData.QuadVertexBufferPtr->EntityID = entityID;
			s_Renderer2DData.QuadVertexBufferPtr++;
		}

		s_Renderer2DData.QuadIndexCount += 6;

		s_Renderer2DData.Stats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x,position.y,0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		DrawQuad(transform, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x,position.y,0.0f }, size, rotation, texture, tilingFactor, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		DrawQuad(transform, texture, tilingFactor, color);
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness /*= 1.0f*/, float fade /*= 0.005f*/, int entityID /*= -1*/)
	{
		// 这里注释是因为，circle一般不会超。。。
		// TODO: implement for circles
		// if (s_Renderer2DData.QuadIndexCount >= Renderer2DData::MaxIndices)
		// 	NextBatch();

		for (size_t i = 0; i < 4; i++)
		{
			s_Renderer2DData.CircleVertexBufferPtr->WorldPosition = transform * s_Renderer2DData.QuadVertexPosition[i];
			s_Renderer2DData.CircleVertexBufferPtr->LocalPosition = s_Renderer2DData.QuadVertexPosition[i] * 2.0f; // x,y取值范围[-1, 1]
			s_Renderer2DData.CircleVertexBufferPtr->Color = color;
			s_Renderer2DData.CircleVertexBufferPtr->Thickness = thickness;
			s_Renderer2DData.CircleVertexBufferPtr->Fade = fade;
			s_Renderer2DData.CircleVertexBufferPtr->EntityID = entityID;
			s_Renderer2DData.CircleVertexBufferPtr++;
		}

		s_Renderer2DData.CircleIndexCount += 6;

		s_Renderer2DData.Stats.QuadCount++;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int entityID)
	{
		s_Renderer2DData.LineVertexBufferPtr->Position = p0;
		s_Renderer2DData.LineVertexBufferPtr->Color = color;
		s_Renderer2DData.LineVertexBufferPtr->EntityID = entityID;
		s_Renderer2DData.LineVertexBufferPtr++;

		s_Renderer2DData.LineVertexBufferPtr->Position = p1;
		s_Renderer2DData.LineVertexBufferPtr->Color = color;
		s_Renderer2DData.LineVertexBufferPtr->EntityID = entityID;
		s_Renderer2DData.LineVertexBufferPtr++;

		s_Renderer2DData.LineVertexCount += 2;
	}

	// 根据一点中心位置确定4个点的位置绘制rect
	void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID)
	{
		// position是中心位置
		glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);// 左下角
		glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);// 右下角
		glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);// 右上角
		glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);// 左上角

		DrawLine(p0, p1, color, entityID);
		DrawLine(p1, p2, color, entityID);
		DrawLine(p2, p3, color, entityID);
		DrawLine(p3, p0, color, entityID);
	}

	// 根据实体的transform确定顶点位置再绘制
	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
			lineVertices[i] = transform * s_Renderer2DData.QuadVertexPosition[i]; // quad的顶点位置正好是rect的顶点位置

		DrawLine(lineVertices[0], lineVertices[1], color, entityID);
		DrawLine(lineVertices[1], lineVertices[2], color, entityID);
		DrawLine(lineVertices[2], lineVertices[3], color, entityID);
		DrawLine(lineVertices[3], lineVertices[0], color, entityID);
	}

	/*
	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x,position.y,0.0f }, size, rotation, subTexture, tilingFactor, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, float tilingFactor, const glm::vec4& color)
	{
		const glm::vec2* textureCoords = subTexture->GetTexCoords();
		const Ref<Texture2D> texture = subTexture->GetTexture();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		if (s_Renderer2DData.QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;

		// 遍历纹理，是否已注入
		for (uint32_t i = 1; i < s_Renderer2DData.TextureSlotIndex; i++)
		{
			if (*s_Renderer2DData.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		//未注入，将texture注入，索引为TextureSlotIndex（从1开始计数）
		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Renderer2DData.TextureSlotIndex;
			s_Renderer2DData.TextureSlots[s_Renderer2DData.TextureSlotIndex] = texture;
			s_Renderer2DData.TextureSlotIndex++;
		}
		for (uint32_t i = 0; i < 4; i++) {
			s_Renderer2DData.QuadVertexBufferPtr->Position = transform * s_Renderer2DData.QuadVertexPosition[i];
			s_Renderer2DData.QuadVertexBufferPtr->Color = color;
			s_Renderer2DData.QuadVertexBufferPtr->TexCoords = textureCoords[i];
			s_Renderer2DData.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_Renderer2DData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Renderer2DData.QuadVertexBufferPtr++;
		}

		s_Renderer2DData.QuadIndexCount += 6;

		s_Renderer2DData.Stats.QuadCount++;
	}
	*/
	void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
	{
		if(src.Texture)
			DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
		else
			DrawQuad(transform, src.Color, entityID);
	}

	float Renderer2D::GetLineWidth()
	{
		return s_Renderer2DData.LineWidth;
	}

	void Renderer2D::SetLineWidth(float width)
	{
		s_Renderer2DData.LineWidth = width;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Renderer2DData.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Renderer2DData.Stats;
	}

}