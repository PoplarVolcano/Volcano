#include "volpch.h"
#include "Cylinder.h"

#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	Cylinder::Cylinder()
	{

		const float PI = 3.14159265359f;

		const uint32_t sectorCount = 36;

		// 侧面 2 * (sectorCount + 1) + 顶部 (sectorCount + 2) + 底部 (sectorCount + 2) = 4 * SectorCount + 6
		m_VertexSize = 4 * sectorCount + 6;
		// 侧面 6 * sectorCount + 顶部 3 * sectorCount + 底部 3 * sectorCount = 12*SectorCount
		m_IndexSize = 12 * sectorCount;

		m_Vertices.resize(m_VertexSize);
		m_Indices.resize(m_IndexSize);

		// 扇形数量
		const float radius = 0.5f; // 柱半径
		const float height = 2.0f; // 柱高
		const float reciprocalSector = 1.0f / (float)sectorCount; // 扇面数倒数
		const float sectorAngle = 2.0f * PI * reciprocalSector;   // 扇区角度

		uint32_t vertexIndex = 0;
		uint32_t indexIndex = 0;

		// 侧面顶点（条带方式）,侧面顶点顺序：top0, bot0, top1, bot1, ... topN, botN （N = SectorCount）
		for (uint32_t i = 0; i != sectorCount + 1; ++i)
		{
			//float angle = (float)i * sectorAngle; // 经度角，从x轴正方形为0度开始
			float angle = (float)i * sectorAngle + PI * 0.5f;   // 经度角，从z轴负方形为0度开始
			float cosA = std::cos(angle);
			float sinA = std::sin(angle);
			glm::vec3 normal = glm::vec3(cosA, 0.0f, -sinA);
			glm::vec3 tangent = glm::vec3(-sinA, 0.0f, cosA);

			// 顶部圆面顶点
			// z轴正方向为接近摄像机方向， position为顺时针，这里取反，让position为逆时针
			m_Vertices[vertexIndex].position  = glm::vec3(radius * cosA, height * 0.5f, -radius * sinA);
			m_Vertices[vertexIndex].texCoord  = glm::vec2((float)i * reciprocalSector, 1.0f);
			m_Vertices[vertexIndex].normal    = normal;
			m_Vertices[vertexIndex].tangent   = tangent; // 环绕方向
			m_Vertices[vertexIndex].bitangent = glm::cross(normal, tangent);
			vertexIndex++;

			// 底部圆面顶点
			m_Vertices[vertexIndex].position  = glm::vec3(radius * cosA, -height * 0.5f, -radius * sinA);
			m_Vertices[vertexIndex].texCoord  = glm::vec2((float)i * reciprocalSector, 0.0f);
			m_Vertices[vertexIndex].normal    = normal;
			m_Vertices[vertexIndex].tangent   = tangent; // 环绕方向
			m_Vertices[vertexIndex].bitangent = glm::cross(normal, tangent);
			vertexIndex++;
		}


		// 侧面索引，每个四边形分为两个三角形：(top_i, bot_i, top_{i+1}) 和 (bot_i, bot_{i+1}, top_{i+1})
		// 0 1 2 / 1 3 2 / 2 3 4 / 3 5 4 / 4 5 6 / 5 7 6
		// 
		// 0  2  4  6
		// 
		// 1  3  5  7
		for (uint32_t i = 0; i != sectorCount; ++i)
		{
			uint32_t base = i * 2; // 每行两个顶点：top, bot

			m_Indices[indexIndex++] = base + 0; // top_i
			m_Indices[indexIndex++] = base + 1; // bot_i
			m_Indices[indexIndex++] = base + 2; // top_{i+1}

			m_Indices[indexIndex++] = base + 1; // bot_i
			m_Indices[indexIndex++] = base + 3; // bot_{i+1}
			m_Indices[indexIndex++] = base + 2; // top_{i+1}
		}

		// 顶部圆面圆心
		uint32_t centerTopIndex = vertexIndex;
		m_Vertices[centerTopIndex].position  = glm::vec3(0.0f, height * 0.5f, 0.0f);
		m_Vertices[centerTopIndex].texCoord  = glm::vec2(0.5f, 0.5f);
		m_Vertices[centerTopIndex].normal    = glm::vec3(0.0f, 1.0f, 0.0f);
		m_Vertices[centerTopIndex].tangent   = glm::vec3(1.0f, 0.0f, 0.0f);
		m_Vertices[centerTopIndex].bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
		vertexIndex++;

		// 顶部圆面边缘顶点
		for (uint32_t i = 0; i != sectorCount + 1; ++i)
		{
			float angle = (float)i * sectorAngle;
			float cosA = cos(angle);
			float sinA = sin(angle);
			m_Vertices[vertexIndex].position  = glm::vec3(radius * cosA, height * 0.5f, -radius * sinA);
			m_Vertices[vertexIndex].texCoord  = glm::vec2(0.5f + 0.5f * cosA, 0.5f + 0.5f * sinA);
			m_Vertices[vertexIndex].normal    = glm::vec3(0.0f, 1.0f, 0.0f);
			m_Vertices[vertexIndex].tangent   = glm::vec3(-sinA, 0.0f, cosA);
			m_Vertices[vertexIndex].bitangent = glm::cross(m_Vertices[vertexIndex].normal, m_Vertices[vertexIndex].tangent);
			vertexIndex++;
		}

		// 顶部圆面索引
		for (uint32_t i = 0; i != sectorCount; ++i) {
			uint32_t edge0 = centerTopIndex + 1 + i;
			uint32_t edge1 = centerTopIndex + 1 + (i + 1) % sectorCount;
			m_Indices[indexIndex++] = centerTopIndex;
			m_Indices[indexIndex++] = edge0;
			m_Indices[indexIndex++] = edge1;
		}


		// 底部圆面圆心
		uint32_t centerBotIndex = vertexIndex;
		m_Vertices[centerBotIndex].position  = glm::vec3(0.0f, -height * 0.5f, 0.0f);
		m_Vertices[centerBotIndex].texCoord  = glm::vec2(0.5f, 0.5f);
		m_Vertices[centerBotIndex].normal    = glm::vec3(0.0f, -1.0f, 0.0f);
		m_Vertices[centerBotIndex].tangent   = glm::vec3(1.0f, 0.0f, 0.0f);
		m_Vertices[centerBotIndex].bitangent = glm::vec3(0.0f, 0.0f, -1.0f);
		vertexIndex++;

		// 底部圆面边缘顶点
		for (uint32_t i = 0; i != sectorCount + 1; ++i)
		{
			float angle = (float)i * sectorAngle;
			float cosA = cos(angle);
			float sinA = sin(angle);
			m_Vertices[vertexIndex].position  = glm::vec3(radius * cosA, -height * 0.5f, -radius * sinA);
			m_Vertices[vertexIndex].texCoord  = glm::vec2(0.5f + 0.5f * cosA, 0.5f + 0.5f * sinA);
			m_Vertices[vertexIndex].normal    = glm::vec3(0.0f, -1.0f, 0.0f);
			m_Vertices[vertexIndex].tangent   = glm::vec3(-sinA, 0.0f, cosA);
			m_Vertices[vertexIndex].bitangent = glm::cross(m_Vertices[vertexIndex].normal, m_Vertices[vertexIndex].tangent);
			vertexIndex++;
		}

		// 顶部圆面索引
		for (uint32_t i = 0; i < sectorCount; ++i) {
			uint32_t edge0 = centerBotIndex + 1 + i;
			uint32_t edge1 = centerBotIndex + 1 + (i + 1) % sectorCount;
			// 注意底部圆面需要翻转绕序以保持法线朝外（逆时针从下方看）
			m_Indices[indexIndex++] = centerBotIndex;
			m_Indices[indexIndex++] = edge1;
			m_Indices[indexIndex++] = edge0;
		}

		m_VertexArray = VertexArray::Create();
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(MeshVertex));
		m_VertexBuffer->SetLayout(m_BufferLayout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Indices.data(), m_IndexSize);
		m_VertexArray->SetIndexBuffer(indexBuffer);
	}

	Cylinder::~Cylinder()
	{
	}

	void Cylinder::FlushInstances()
	{
		Renderer::DrawIndexInstanced(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount(), m_InstanceDataList.size());
	}
}