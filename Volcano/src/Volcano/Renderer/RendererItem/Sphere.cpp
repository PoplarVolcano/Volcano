#include "volpch.h"
#include "Sphere.h"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Buffer.h"
#include "Volcano/Renderer/Shader.h"
#include "Volcano/Renderer/Texture.h"
#include "glm/glm.hpp"
#include <glm/gtx/transform.hpp>

namespace Volcano {

	struct SphereVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		glm::vec3 Normal;

		// Editor-only
		int EntityID;
	};

	struct SphereData
	{
        static const uint32_t X_SEGMENTS = 64; // β角
        static const uint32_t Y_SEGMENTS = 64; // α角
		static const uint32_t VertexCount = (X_SEGMENTS + 1) * (Y_SEGMENTS + 1);
		static const uint32_t IndicesCount = (X_SEGMENTS + 1) * Y_SEGMENTS * 2;
		static const uint32_t MaxSpheres = 2000;
		static const uint32_t MaxVertices = MaxSpheres * VertexCount;
		static const uint32_t MaxIndices  = MaxSpheres * (X_SEGMENTS + 1) * Y_SEGMENTS * 2;
		static const uint32_t MaxTextureSlots = 32;// RenderCaps

		Ref<VertexArray>  SphereVertexArray;
		Ref<VertexBuffer> SphereVertexBuffer;
		Ref<Shader>       SphereShader;
		Ref<Texture2D>    WhiteTexture;
		Ref<TextureCube>  IrradianceMap;
		Ref<TextureCube>  PrefilterMap;
		Ref<Texture2D>    BRDFLUT;

		uint32_t SphereIndexCount = 0;
        SphereVertex* SphereVertexBufferBase = nullptr;
        SphereVertex* SphereVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;// 0 = white texture

        glm::vec3 Positions[VertexCount];
        glm::vec3 Normals[VertexCount];
        glm::vec2 UV[VertexCount];
	};

	static SphereData s_SphereData;

	void Sphere::Init()
    {
        s_SphereData.SphereVertexArray = VertexArray::Create();


        const unsigned int X_SEGMENTS = SphereData::X_SEGMENTS;
        const unsigned int Y_SEGMENTS = SphereData::Y_SEGMENTS;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; x++)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; y++)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                s_SphereData.Positions[y + x * (X_SEGMENTS + 1)] = glm::vec3(xPos, yPos, zPos);
                s_SphereData.Normals[y + x * (X_SEGMENTS + 1)] = glm::vec3(xPos, yPos, zPos);
                s_SphereData.UV[y + x * (X_SEGMENTS + 1)] = glm::vec2(xSegment, ySegment);
            }
        }

        s_SphereData.SphereVertexBuffer = VertexBuffer::Create(s_SphereData.MaxVertices * sizeof(SphereVertex));
        s_SphereData.SphereVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position"     },
            { ShaderDataType::Float2, "a_TexCoords"    },
            { ShaderDataType::Float3, "a_Normal"       },
            { ShaderDataType::Int,    "a_EntityID"     }
            });
        s_SphereData.SphereVertexArray->AddVertexBuffer(s_SphereData.SphereVertexBuffer);
		s_SphereData.SphereVertexBufferBase = new SphereVertex[s_SphereData.MaxVertices];


		std::vector<unsigned int> indices;
		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			// 一奇一偶为一个矩形，[0,0],[0,1],[1,1],[1,0]
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		uint32_t* sphereIndicesBuffer = new uint32_t[s_SphereData.MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_SphereData.MaxIndices; i += s_SphereData.IndicesCount)
		{
			for (uint32_t j = 0; j < s_SphereData.IndicesCount; j++)
				sphereIndicesBuffer[i + j] = offset + indices[j];
			offset += s_SphereData.VertexCount;
		}
		Ref<IndexBuffer> sphereIndexBuffer = IndexBuffer::Create(sphereIndicesBuffer, s_SphereData.MaxIndices);;
		s_SphereData.SphereVertexArray->SetIndexBuffer(sphereIndexBuffer);
		delete[] sphereIndicesBuffer;	// cpu上传到gpu上了可以删除cpu的索引数据块了
		

        s_SphereData.WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_SphereData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		s_SphereData.TextureSlots[0] = s_SphereData.WhiteTexture;

        //Renderer::GetShaderLibrary()->Load("assets/shaders/3D/Sphere.glsl");
		//s_SphereData.SphereShader = Renderer::GetShaderLibrary()->Get("Sphere");
		Renderer::GetShaderLibrary()->Load("assets/shaders/3D/PBR.glsl");
		s_SphereData.SphereShader = Renderer::GetShaderLibrary()->Get("PBR");
		glm::vec2 temp = glm::vec2(0.0);
    }

	void Sphere::Shutdown()
	{
		delete[] s_SphereData.SphereVertexBufferBase;
	}

	void Sphere::BeginScene(const Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction)
	{
		StartBatch();
	}

	void Sphere::EndScene(RenderType type)
	{
		Flush(type);
	}

	void Sphere::Flush(RenderType type)
	{
		if (s_SphereData.SphereIndexCount)
		{
			// 不加uint8_t转化会得到元素数量, 加uint8_t返回以char为单位占用多少元素
			uint32_t dataSize = (uint32_t)((uint8_t*)s_SphereData.SphereVertexBufferPtr - (uint8_t*)s_SphereData.SphereVertexBufferBase);
			s_SphereData.SphereVertexBuffer->SetData(s_SphereData.SphereVertexBufferBase, dataSize);

			// Bind textures
			for (uint32_t i = 0; i < s_SphereData.TextureSlotIndex; i++)
				s_SphereData.TextureSlots[i]->Bind(i);
			

			if (s_SphereData.IrradianceMap)
				s_SphereData.IrradianceMap->Bind(6);
			if (s_SphereData.PrefilterMap)
				s_SphereData.PrefilterMap->Bind(7);
			if (s_SphereData.BRDFLUT)
				s_SphereData.BRDFLUT->Bind(25);

			/*
			switch (type)
			{
			case RenderType::SHADOW_DIRECTIONALLIGHT:
				Renderer::GetShaderLibrary()->Get("ShadowMappingDepth")->Bind();
				break;
			case RenderType::SHADOW_POINTLIGHT:
				Renderer::GetShaderLibrary()->Get("PointShadowsDepth")->Bind();
				break;
			case RenderType::SHADOW_SPOTLIGHT:
				Renderer::GetShaderLibrary()->Get("SpotShadowDepth")->Bind();
				break;
			case RenderType::G_BUFFER:
				Renderer::GetShaderLibrary()->Get("GBuffer")->Bind();
				break;
			case RenderType::DEFERRED_SHADING:
				Renderer::GetShaderLibrary()->Get("DeferredShading")->Bind();
				break;
			case RenderType::NORMAL:
				s_SphereData.SphereShader->Bind();
				break;
			default:
				VOL_CORE_ASSERT(0);
			}
			*/
			if (type == RenderType::NORMAL)
			{
				s_SphereData.SphereShader->Bind();
				Renderer::DrawStripIndexed(s_SphereData.SphereVertexArray, s_SphereData.SphereIndexCount);
			}
		}
	}

	void Sphere::StartBatch()
	{
		s_SphereData.SphereIndexCount = 0;
		s_SphereData.SphereVertexBufferPtr = s_SphereData.SphereVertexBufferBase;

		s_SphereData.TextureSlotIndex = 1;
	}

	void Sphere::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Sphere::DrawSphere(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawSphere({ position.x,position.y,0.0f }, size, color);
	}

	void Sphere::DrawSphere(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		glm::mat3 normalTransform = glm::mat3(transpose(inverse(transform)));
		DrawSphere(transform, normalTransform, color);
	}

	void Sphere::DrawSphere(const glm::mat4& transform, const glm::mat3& normalTransform, const glm::vec4& color, int entityID)
	{
		if (s_SphereData.SphereIndexCount >= SphereData::MaxIndices)
			NextBatch();

		for (uint32_t i = 0; i < s_SphereData.VertexCount; i++) {
			s_SphereData.SphereVertexBufferPtr->Position = transform * glm::vec4(s_SphereData.Positions[i], 1.0f);
			s_SphereData.SphereVertexBufferPtr->TexCoords = s_SphereData.UV[i];
			s_SphereData.SphereVertexBufferPtr->Normal = normalTransform * s_SphereData.Normals[i];
			s_SphereData.SphereVertexBufferPtr->EntityID = entityID;
			s_SphereData.SphereVertexBufferPtr++;
		}

		s_SphereData.SphereIndexCount += s_SphereData.IndicesCount;
	}

	void Sphere::DrawSphere(const glm::mat4& transform, const glm::mat3& normalTransform,
		const Ref<Texture2D>& albedo, const Ref<Texture2D>& normal, const Ref<Texture2D>& metallic,
		const Ref<Texture2D>& roughness, const Ref<Texture2D>& AO, const glm::vec4& color, int entityID) 
	{
		if (s_SphereData.SphereIndexCount >= SphereData::MaxIndices)
			NextBatch();

		
		float albedoIndex = 0.0f;
		float normalIndex = 0.0f;
		float metallicIndex = 0.0f;
		float roughnessIndex = 0.0f;
		float AOIndex = 0.0f;

		if (albedo)
		{
			// 遍历纹理，albedo是否已注入纹理通道TextureSlot
			for (uint32_t i = 1; i < s_SphereData.TextureSlotIndex; i++)
			{
				// 已注入，将albedoIndex设为该纹理
				if (*s_SphereData.TextureSlots[i].get() == *albedo.get())
				{
					albedoIndex = (float)i;
					break;
				}
			}

			// albedo未注入，将albedo注入纹理通道TextureSlot，索引为TextureSlotIndex（从1开始计数）
			if (albedoIndex == 0.0f)
			{
				if (s_SphereData.TextureSlotIndex >= SphereData::MaxTextureSlots)
					NextBatch();
				albedoIndex = (float)s_SphereData.TextureSlotIndex;
				s_SphereData.TextureSlots[s_SphereData.TextureSlotIndex] = albedo;
				s_SphereData.TextureSlotIndex++;
			}
		}

		if (normal)
		{
			for (uint32_t i = 1; i < s_SphereData.TextureSlotIndex; i++)
			{
				if (*s_SphereData.TextureSlots[i].get() == *normal.get())
				{
					normalIndex = (float)i;
					break;
				}
			}
			if (normalIndex == 0.0f)
			{
				if (s_SphereData.TextureSlotIndex >= SphereData::MaxTextureSlots)
					NextBatch();
				normalIndex = (float)s_SphereData.TextureSlotIndex;
				s_SphereData.TextureSlots[s_SphereData.TextureSlotIndex] = normal;
				s_SphereData.TextureSlotIndex++;
			}
		}

		if (metallic)
		{
			for (uint32_t i = 1; i < s_SphereData.TextureSlotIndex; i++)
			{
				if (*s_SphereData.TextureSlots[i].get() == *metallic.get())
				{
					metallicIndex = (float)i;
					break;
				}
			}
			if (metallicIndex == 0.0f)
			{
				if (s_SphereData.TextureSlotIndex >= SphereData::MaxTextureSlots)
					NextBatch();
				metallicIndex = (float)s_SphereData.TextureSlotIndex;
				s_SphereData.TextureSlots[s_SphereData.TextureSlotIndex] = metallic;
				s_SphereData.TextureSlotIndex++;
			}
		}

		if (roughness)
		{
			for (uint32_t i = 1; i < s_SphereData.TextureSlotIndex; i++)
			{
				if (*s_SphereData.TextureSlots[i].get() == *roughness.get())
				{
					roughnessIndex = (float)i;
					break;
				}
			}
			if (roughnessIndex == 0.0f)
			{
				if (s_SphereData.TextureSlotIndex >= SphereData::MaxTextureSlots)
					NextBatch();
				roughnessIndex = (float)s_SphereData.TextureSlotIndex;
				s_SphereData.TextureSlots[s_SphereData.TextureSlotIndex] = roughness;
				s_SphereData.TextureSlotIndex++;
			}
		}

		if (AO)
		{
			for (uint32_t i = 1; i < s_SphereData.TextureSlotIndex; i++)
			{
				if (*s_SphereData.TextureSlots[i].get() == *AO.get())
				{
					AOIndex = (float)i;
					break;
				}
			}
			if (AOIndex == 0.0f)
			{
				if (s_SphereData.TextureSlotIndex >= SphereData::MaxTextureSlots)
					NextBatch();
				AOIndex = (float)s_SphereData.TextureSlotIndex;
				s_SphereData.TextureSlots[s_SphereData.TextureSlotIndex] = AO;
				s_SphereData.TextureSlotIndex++;
			}
		}

		for (uint32_t i = 0; i < s_SphereData.VertexCount; i++) {
			s_SphereData.SphereVertexBufferPtr->Position = transform * glm::vec4(s_SphereData.Positions[i], 1.0f);
			// 模型矩阵左上角3x3部分的逆矩阵的转置矩阵，用于解决不等比缩放导致的法向量不垂直于平面
			s_SphereData.SphereVertexBufferPtr->TexCoords = s_SphereData.UV[i];
			s_SphereData.SphereVertexBufferPtr->Normal = normalTransform * s_SphereData.Normals[i];
			s_SphereData.SphereVertexBufferPtr->EntityID = entityID;
			s_SphereData.SphereVertexBufferPtr++;
		}

		s_SphereData.SphereIndexCount += s_SphereData.IndicesCount;
	}

	void Sphere::DrawSphere(const glm::mat4& transform, const glm::mat3& normalTransform, SphereRendererComponent& src, int entityID)
	{
		if (src.Albedo)
			DrawSphere(transform, normalTransform, src.Albedo, src.Normal, src.Metallic, src.Roughness, src.AO, src.Color, entityID);
		else
			DrawSphere(transform, normalTransform, src.Color, entityID);
	}
	void Sphere::SetIrradianceMap(const Ref<TextureCube> irradianceMap)
	{
		s_SphereData.IrradianceMap = irradianceMap;
	}
	void Sphere::SetPrefilterMap(const Ref<TextureCube> prefilterMap)
	{
		s_SphereData.PrefilterMap = prefilterMap;
	}
	void Sphere::SetBRDFLUT(const Ref<Texture2D> BRDFLUT)
	{
		s_SphereData.BRDFLUT = BRDFLUT;
	}
}