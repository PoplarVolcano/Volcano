#include "volpch.h"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/Renderer3D.h"
#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Shader.h"
#include "Volcano/Renderer/UniformBuffer.h"
#include "Volcano/Renderer/Light.h"

namespace Volcano {

	struct CubeVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec4 Color;
		glm::vec2 TexCoords;
		float DiffuseIndex;
		float SpecularIndex;

		// Editor-only
		int EntityID;
	};

	struct CubeData
	{
		static const uint32_t MaxCubes = 20000;
		static const uint32_t MaxVertices = MaxCubes * 36;
		static const uint32_t MaxIndices = MaxCubes * 36;
		static const uint32_t MaxTextureSlots = 32;// RenderCaps

		Ref<VertexArray>  CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<Shader>       CubeShader;
		Ref<Texture2D>    WhiteTexture;

		uint32_t CubeIndexCount = 0;
		CubeVertex* CubeVertexBufferBase = nullptr;
		CubeVertex* CubeVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;// 0 = white texture

		glm::vec3 CubeVertexPosition[36];
		glm::vec3 CubeNormal[6];
	};
	static CubeData s_CubeData;

	void Renderer3D::Init()
	{
		// Cube
		s_Renderer3DData.CubeVertexArray = VertexArray::Create();
		s_Renderer3DData.CubeVertexBuffer = VertexBuffer::Create(s_Renderer3DData.MaxVertices * sizeof(CubeVertex));
		s_Renderer3DData.CubeVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float3, "a_Normal"       },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float2, "a_TexCoords"    },
			{ ShaderDataType::Float,  "a_DiffuseIndex" },
			{ ShaderDataType::Float,  "a_SpecularIndex"},
			{ ShaderDataType::Int,    "a_EntityID"     }
			});
		s_Renderer3DData.CubeVertexArray->AddVertexBuffer(s_Renderer3DData.CubeVertexBuffer);
		s_Renderer3DData.CubeVertexBufferBase = new CubeVertex[s_Renderer3DData.MaxVertices];

		const uint32_t indicesSize = 36;

		uint32_t* cubeIndicesBuffer = new uint32_t[s_Renderer3DData.MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Renderer3DData.MaxIndices; i += indicesSize)
		{
			for (uint32_t j = 0; j < 36; j++)
				cubeIndicesBuffer[i + j] = offset + j;
			offset += 36;
		}
		Ref<IndexBuffer> cubeIndexBuffer = IndexBuffer::Create(cubeIndicesBuffer, s_Renderer3DData.MaxIndices);;
		s_Renderer3DData.CubeVertexArray->SetIndexBuffer(cubeIndexBuffer);
		delete[] cubeIndicesBuffer;	// cpu上传到gpu上了可以删除cpu的索引数据块了

		s_Renderer3DData.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Renderer3DData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[s_Renderer3DData.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Renderer3DData.MaxTextureSlots; i++)
			samplers[i] = i;

		Renderer::GetShaderLibrary()->Load("assets/shaders/Renderer3D_Cube.glsl");
		s_Renderer3DData.CubeShader = Renderer::GetShaderLibrary()->Get("Renderer3D_Cube");

		s_Renderer3DData.TextureSlots[0] = s_Renderer3DData.WhiteTexture;
		//背
		s_Renderer3DData.CubeVertexPosition[0] = { -0.5f, -0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[1] = { 0.5f,  0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[2] = { 0.5f, -0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[3] = { 0.5f,  0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[4] = { -0.5f, -0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[5] = { -0.5f,  0.5f, -0.5f };
		//正
		s_Renderer3DData.CubeVertexPosition[6] = { -0.5f, -0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[7] = { 0.5f, -0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[8] = { 0.5f,  0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[9] = { 0.5f,  0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[10] = { -0.5f,  0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[11] = { -0.5f, -0.5f,  0.5f };
		//左
		s_Renderer3DData.CubeVertexPosition[12] = { -0.5f,  0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[13] = { -0.5f,  0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[14] = { -0.5f, -0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[15] = { -0.5f, -0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[16] = { -0.5f, -0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[17] = { -0.5f,  0.5f,  0.5f };
		//右
		s_Renderer3DData.CubeVertexPosition[18] = { 0.5f,  0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[19] = { 0.5f, -0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[20] = { 0.5f,  0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[21] = { 0.5f, -0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[22] = { 0.5f,  0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[23] = { 0.5f, -0.5f,  0.5f };
		//下
		s_Renderer3DData.CubeVertexPosition[24] = { -0.5f, -0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[25] = { 0.5f, -0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[26] = { 0.5f, -0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[27] = { 0.5f, -0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[28] = { -0.5f, -0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[29] = { -0.5f, -0.5f, -0.5f };
		//上
		s_Renderer3DData.CubeVertexPosition[30] = { -0.5f,  0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[31] = { 0.5f,  0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[32] = { 0.5f,  0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[33] = { 0.5f,  0.5f,  0.5f };
		s_Renderer3DData.CubeVertexPosition[34] = { -0.5f,  0.5f, -0.5f };
		s_Renderer3DData.CubeVertexPosition[35] = { -0.5f,  0.5f,  0.5f };

		s_Renderer3DData.CubeNormal[0] = glm::vec3(0.0f, 0.0f, -1.0f);//背
		s_Renderer3DData.CubeNormal[1] = glm::vec3(0.0f, 0.0f, 1.0f);//正
		s_Renderer3DData.CubeNormal[2] = glm::vec3(-1.0f, 0.0f, 0.0f);//左
		s_Renderer3DData.CubeNormal[3] = glm::vec3(1.0f, 0.0f, 0.0f);//右
		s_Renderer3DData.CubeNormal[4] = glm::vec3(0.0f, -1.0f, 0.0f);//下
		s_Renderer3DData.CubeNormal[5] = glm::vec3(0.0f, 1.0f, 0.0f);//上
		/*
		// 正面
		s_Renderer3DData.CubeNormal[0] = glm::cross(
			s_Renderer3DData.CubeVertexPosition[1] - s_Renderer3DData.CubeVertexPosition[0],
			s_Renderer3DData.CubeVertexPosition[3] - s_Renderer3DData.CubeVertexPosition[0]);

		// 背面
		s_Renderer3DData.CubeNormal[1] = glm::cross(
			s_Renderer3DData.CubeVertexPosition[4] - s_Renderer3DData.CubeVertexPosition[5],
			s_Renderer3DData.CubeVertexPosition[6] - s_Renderer3DData.CubeVertexPosition[5]);

		// 左面
		s_Renderer3DData.CubeNormal[2] = glm::cross(
			s_Renderer3DData.CubeVertexPosition[0] - s_Renderer3DData.CubeVertexPosition[4],
			s_Renderer3DData.CubeVertexPosition[7] - s_Renderer3DData.CubeVertexPosition[4]);

		// 右面
		s_Renderer3DData.CubeNormal[3] = glm::cross(
			s_Renderer3DData.CubeVertexPosition[5] - s_Renderer3DData.CubeVertexPosition[1],
			s_Renderer3DData.CubeVertexPosition[2] - s_Renderer3DData.CubeVertexPosition[1]);

		// 下面
		s_Renderer3DData.CubeNormal[4] = glm::cross(
			s_Renderer3DData.CubeVertexPosition[5] - s_Renderer3DData.CubeVertexPosition[4],
			s_Renderer3DData.CubeVertexPosition[0] - s_Renderer3DData.CubeVertexPosition[4]);

		// 上面
		s_Renderer3DData.CubeNormal[5] = glm::cross(
			s_Renderer3DData.CubeVertexPosition[2] - s_Renderer3DData.CubeVertexPosition[3],
			s_Renderer3DData.CubeVertexPosition[7] - s_Renderer3DData.CubeVertexPosition[3]);
			*/

		s_CameraUniformBuffer = UniformBuffer::Create(4 * 4 * sizeof(float), 0);
		s_CameraPositionUniformBuffer = UniformBuffer::Create(4 * sizeof(float), 1);
		s_DirectionalLightUniformBuffer = UniformBuffer::Create((4 + 4 + 4 + 4) * sizeof(float), 2);
		s_PointLightUniformBuffer = UniformBuffer::Create((4 + 4 + 4 + 3 + 1 + 1 + 1) * sizeof(float), 3);
		s_SpotLightUniformBuffer = UniformBuffer::Create((4 + 4 + 4 + 4 + 3 + 1 + 1 + 1 + 1 + 1) * sizeof(float), 4);
		s_MaterialUniformBuffer = UniformBuffer::Create(sizeof(float), 5);
	}

	void Renderer3D::Shutdown()
	{
		delete[] s_Renderer3DData.CubeVertexBufferBase;
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction)
	{
		s_CameraBuffer.viewProjection = camera.GetProjection() * glm::inverse(transform);
		s_CameraUniformBuffer->SetData(&s_CameraBuffer.viewProjection, sizeof(glm::mat4));

		// transform最后一列前三个位置为translate
		s_CameraPositionBuffer.CameraPosition = position;
		s_CameraPositionUniformBuffer->SetData(&s_CameraPositionBuffer.CameraPosition, sizeof(glm::vec3));

		s_DirectionalLightBuffer.direction = glm::vec3(-1.0f, -1.0f, -1.0f);
		s_DirectionalLightBuffer.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
		s_DirectionalLightBuffer.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		s_DirectionalLightBuffer.specular = glm::vec3(0.5f, 0.5f, 0.5f);
		s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.direction, sizeof(glm::vec3));
		s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.ambient, sizeof(glm::vec3), 4 * sizeof(float));
		s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.diffuse, sizeof(glm::vec3), (4 + 4) * sizeof(float));
		s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.specular, sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));


		s_PointLightBuffer.position = glm::vec3(1.0f, 1.0f, 1.0f);
		s_PointLightBuffer.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
		s_PointLightBuffer.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
		s_PointLightBuffer.specular = glm::vec3(1.0f, 1.0f, 1.0f);
		s_PointLightBuffer.constant = 1.0f;
		s_PointLightBuffer.linear = 0.09f;
		s_PointLightBuffer.quadratic = 0.032f;
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.position, sizeof(glm::vec3));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.ambient, sizeof(glm::vec3), 4 * sizeof(float));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.diffuse, sizeof(glm::vec3), (4 + 4) * sizeof(float));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.specular, sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.constant, sizeof(float), (4 + 4 + 4 + 3) * sizeof(float));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.linear, sizeof(float), (4 + 4 + 4 + 3 + 1) * sizeof(float));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.quadratic, sizeof(float), (4 + 4 + 4 + 3 + 1 + 1) * sizeof(float));


		s_SpotLightBuffer.position = position;
		s_SpotLightBuffer.direction = direction;
		s_SpotLightBuffer.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
		s_SpotLightBuffer.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		s_SpotLightBuffer.specular = glm::vec3(1.0f, 1.0f, 1.0f);
		s_SpotLightBuffer.constant = 1.0f;
		s_SpotLightBuffer.linear = 0.09f;
		s_SpotLightBuffer.quadratic = 0.032f;
		s_SpotLightBuffer.cutOff = glm::cos(glm::radians(12.5f));
		s_SpotLightBuffer.outerCutOff = glm::cos(glm::radians(17.5f));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.position, sizeof(glm::vec3));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.direction, sizeof(glm::vec3), 4 * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.ambient, sizeof(glm::vec3), (4 + 4) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.diffuse, sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.specular, sizeof(glm::vec3), (4 + 4 + 4 + 4) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.constant, sizeof(float), (4 + 4 + 4 + 4 + 3) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.linear, sizeof(float), (4 + 4 + 4 + 4 + 3 + 1) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.quadratic, sizeof(float), (4 + 4 + 4 + 4 + 3 + 1 + 1) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.cutOff, sizeof(float), (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.outerCutOff, sizeof(float), (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1 + 1) * sizeof(float));

		s_MaterialBuffer.shininess = 32.0f;
		s_MaterialUniformBuffer->SetData(&s_MaterialBuffer.shininess, sizeof(float));

		StartBatch();
	}

	void Renderer3D::EndScene()
	{
		Flush();
	}

	void Renderer3D::Flush()
	{
		if (s_Renderer3DData.CubeIndexCount)
		{
			// 不加uint8_t转化会得到元素数量, 加uint8_t返回以char为单位占用多少元素
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Renderer3DData.CubeVertexBufferPtr - (uint8_t*)s_Renderer3DData.CubeVertexBufferBase);
			s_Renderer3DData.CubeVertexBuffer->SetData(s_Renderer3DData.CubeVertexBufferBase, dataSize);

			// Bind textures
			for (uint32_t i = 0; i < s_Renderer3DData.TextureSlotIndex; i++)
				s_Renderer3DData.TextureSlots[i]->Bind(i);

			s_Renderer3DData.CubeShader->Bind();
			Renderer::DrawIndexed(s_Renderer3DData.CubeVertexArray, s_Renderer3DData.CubeIndexCount);
		}
	}

	void Renderer3D::StartBatch()
	{
		s_Renderer3DData.CubeIndexCount = 0;
		s_Renderer3DData.CubeVertexBufferPtr = s_Renderer3DData.CubeVertexBufferBase;

		s_Renderer3DData.TextureSlotIndex = 1;
	}

	void Renderer3D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer3D::DrawCube(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawCube({ position.x,position.y,0.0f }, size, color);
	}

	void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		glm::mat3 normalTransform = glm::mat3(transpose(inverse(transform)));
		DrawCube(transform, normalTransform, color);
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const glm::mat3& normalTransform, const glm::vec4& color, int entityID)
	{
		if (s_Renderer3DData.CubeIndexCount >= Renderer3DData::MaxIndices)
			NextBatch();

		float diffuseIndex = 0.0f;
		float specularIndex = 0.0f; // White Texture

		constexpr glm::vec2 textureCoords[] = {
			{0.0f, 0.0f},
			{1.0f, 1.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f},
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f},
			{1.0f, 0.0f},
			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f},
			{1.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{0.0f, 1.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},
			{0.0f, 0.0f} };

		const float tilingFactor = 1.0f;

		// 逆时针注入顶点数据
		// 设置顶点的地址指向注入的地址
		for (uint32_t i = 0; i < 36; i++) {
			s_Renderer3DData.CubeVertexBufferPtr->Position = transform * glm::vec4(s_Renderer3DData.CubeVertexPosition[i], 1.0f);
			// 模型矩阵左上角3x3部分的逆矩阵的转置矩阵，用于解决不等比缩放导致的法向量不垂直于平面
			s_Renderer3DData.CubeVertexBufferPtr->Normal = normalTransform * s_Renderer3DData.CubeNormal[i / 6];
			s_Renderer3DData.CubeVertexBufferPtr->Color = color;
			s_Renderer3DData.CubeVertexBufferPtr->TexCoords = textureCoords[i];
			s_Renderer3DData.CubeVertexBufferPtr->DiffuseIndex = diffuseIndex;
			s_Renderer3DData.CubeVertexBufferPtr->SpecularIndex = specularIndex;
			s_Renderer3DData.CubeVertexBufferPtr->EntityID = entityID;
			s_Renderer3DData.CubeVertexBufferPtr++;
		}

		s_Renderer3DData.CubeIndexCount += 36;
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const glm::mat3& normalTransform, const Ref<Texture2D>& diffuse, const Ref<Texture2D>& specular, const glm::vec4& color, int entityID)
	{
		if (s_Renderer3DData.CubeIndexCount >= Renderer3DData::MaxIndices)
			NextBatch();

		float diffuseIndex = 0.0f;
		float specularIndex = 0.0f;
		constexpr glm::vec2 textureCoords[] = {
			{0.0f, 0.0f},
			{1.0f, 1.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f},
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f},
			{1.0f, 0.0f},
			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f},
			{1.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{0.0f, 1.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},
			{0.0f, 0.0f} };

		if (diffuse)
		{
			// 遍历纹理，diffuse是否已注入纹理通道TextureSlot
			for (uint32_t i = 1; i < s_Renderer3DData.TextureSlotIndex; i++)
			{
				// 已注入，将diffuseIndex设为该纹理
				if (*s_Renderer3DData.TextureSlots[i].get() == *diffuse.get())
				{
					diffuseIndex = (float)i;
					break;
				}
			}

			// diffuse未注入，将diffuse注入纹理通道TextureSlot，索引为TextureSlotIndex（从1开始计数）
			if (diffuseIndex == 0.0f)
			{
				if (s_Renderer3DData.TextureSlotIndex >= Renderer3DData::MaxTextureSlots)
					NextBatch();
				diffuseIndex = (float)s_Renderer3DData.TextureSlotIndex;
				s_Renderer3DData.TextureSlots[s_Renderer3DData.TextureSlotIndex] = diffuse;
				s_Renderer3DData.TextureSlotIndex++;
			}
		}

		if (specular)
		{
			// 遍历纹理，specular是否已注入纹理通道TextureSlot
			for (uint32_t i = 1; i < s_Renderer3DData.TextureSlotIndex; i++)
			{
				// 已注入，将specularIndex设为该纹理
				if (*s_Renderer3DData.TextureSlots[i].get() == *specular.get())
				{
					specularIndex = (float)i;
					break;
				}
			}
			// specular未注入，将specular注入纹理通道TextureSlot，索引为TextureSlotIndex（从1开始计数）
			if (specularIndex == 0.0f)
			{
				if (s_Renderer3DData.TextureSlotIndex >= Renderer3DData::MaxTextureSlots)
					NextBatch();
				specularIndex = (float)s_Renderer3DData.TextureSlotIndex;
				s_Renderer3DData.TextureSlots[s_Renderer3DData.TextureSlotIndex] = specular;
				s_Renderer3DData.TextureSlotIndex++;
			}
		}

		for (uint32_t i = 0; i < 36; i++) {
			s_Renderer3DData.CubeVertexBufferPtr->Position = transform * glm::vec4(s_Renderer3DData.CubeVertexPosition[i], 1.0f);
			s_Renderer3DData.CubeVertexBufferPtr->Normal = normalTransform * s_Renderer3DData.CubeNormal[i / 6];
			s_Renderer3DData.CubeVertexBufferPtr->Color = color;
			s_Renderer3DData.CubeVertexBufferPtr->TexCoords = textureCoords[i];
			s_Renderer3DData.CubeVertexBufferPtr->DiffuseIndex = diffuseIndex;
			s_Renderer3DData.CubeVertexBufferPtr->SpecularIndex = specularIndex;
			s_Renderer3DData.CubeVertexBufferPtr->EntityID = entityID;
			s_Renderer3DData.CubeVertexBufferPtr++;
		}

		s_Renderer3DData.CubeIndexCount += 36;
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const glm::mat3& normalTransform, CubeRendererComponent& crc, int entityID)
	{
		if (crc.Diffuse)
			DrawCube(transform, normalTransform, crc.Diffuse, crc.Specular, crc.Color, entityID);
		else
			DrawCube(transform, normalTransform, crc.Color, entityID);
	}

}