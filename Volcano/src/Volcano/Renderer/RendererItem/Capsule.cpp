#include "volpch.h"
#include "Capsule.h"

#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	Capsule::Capsule()
	{

		const float PI = 3.14159265359f;

		const uint32_t X_SEGMENTS = 64;
		const uint32_t Y_SEGMENTS = 64;
		m_VertexSize = (X_SEGMENTS + 2) * (Y_SEGMENTS + 2);
		m_IndexSize  = (X_SEGMENTS + 2) * (Y_SEGMENTS + 1) * 2;

		m_Vertices.resize(m_VertexSize);
		m_Indices.resize(m_IndexSize);

		const float radius = 0.5f;
		const float reight = 1.0f;
		const float reciprocalXSegments = 1.0f / (float)X_SEGMENTS; // 扇面数倒数
		const float reciprocalYSegments = 1.0f / (float)Y_SEGMENTS; // 扇面数倒数

		uint32_t vertexIndex = 0;
		uint32_t indexIndex = 0;

		// 球面
		// 因为GL_TRIANGLE_STRIP渲染球体要求顶点的行、列数相同
		// 行数因为y轴被分成了2个半球而多了一行，所以列数也应该对应的多1行
		for (uint32_t x = 0; x != X_SEGMENTS + 2; x++)
		{
			for (uint32_t y = 0; y != Y_SEGMENTS / 2 + 1; y++)
			{
				float xSegment = (float)x * reciprocalXSegments;
				float ySegment = (float)y * reciprocalYSegments;
				//float phi = xSegment * 2.0f * PI;           // 经度角 φ，从x轴正方形为0度开始
				float phi = xSegment * 2.0f * PI - PI * 0.5f; // 经度角 φ，从z轴负方形为0度开始
				float theta = ySegment * PI;                  // 纬度角 θ
				float xDir = std::cos(phi) * std::sin(theta);
				float yDir = std::cos(theta);
				float zDir = std::sin(phi) * std::sin(theta);
				float xPos = radius * xDir;
				float yPos = radius * yDir + reight * 0.5f;
				float zPos = radius * zDir;

				uint32_t index = y + x * (Y_SEGMENTS + 2);
				m_Vertices[index].position  = glm::vec3(xPos, yPos, zPos);
				m_Vertices[index].texCoord  = glm::vec2(1.0f - xSegment, 1.0f - ySegment * 0.5f);
				m_Vertices[index].normal    = glm::vec3(xDir, yDir, zDir);
				m_Vertices[index].tangent   = glm::vec3(-std::sin(phi), 0.0f, std::cos(phi));;
				m_Vertices[index].bitangent = glm::vec3(
					std::cos(phi) * std::cos(theta),
					-sin(theta),
					sin(phi) * cos(theta));
			}

			for (uint32_t y = Y_SEGMENTS / 2; y != Y_SEGMENTS + 1; y++)
			{
				float xSegment = (float)x * reciprocalXSegments;
				float ySegment = (float)y * reciprocalYSegments;
				//float phi = xSegment * 2.0f * PI;           // 经度角 φ，从x轴正方形为0度开始
				float phi = xSegment * 2.0f * PI - PI * 0.5f; // 经度角 φ，从z轴负方形为0度开始
				float theta = ySegment * PI;                  // 纬度角 θ
				float xDir = std::cos(phi) * std::sin(theta);
				float yDir = std::cos(theta);
				float zDir = std::sin(phi) * std::sin(theta);
				float xPos = radius * xDir;
				float yPos = radius * yDir - reight * 0.5f;
				float zPos = radius * zDir;

				uint32_t index = (y + 1) + x * (Y_SEGMENTS + 2);
				m_Vertices[index].position  = glm::vec3(xPos, yPos, zPos);
				m_Vertices[index].texCoord  = glm::vec2(1.0f - xSegment, 0.5f - ySegment * 0.5f);
				m_Vertices[index].normal    = glm::vec3(xDir, yDir, zDir);
				m_Vertices[index].tangent   = glm::vec3(-std::sin(phi), 0.0f, std::cos(phi));;
				m_Vertices[index].bitangent = glm::vec3(
					std::cos(phi) * std::cos(theta),
					-sin(theta),
					sin(phi) * cos(theta));
			}
		}

		indexIndex = 0;

		// 是否是奇数行
		// bool isOddRow = false;
		// 是否是偶数行
		bool isEvenRow = true;
		for (uint32_t y = 0; y != Y_SEGMENTS + 1; ++y)
		{
			if (isEvenRow)
			{
				for (uint32_t x = 0; x != X_SEGMENTS + 2; ++x)
				{
					m_Indices[indexIndex++] = y * (X_SEGMENTS + 2) + x;      // 当前行的顶点
					m_Indices[indexIndex++] = (y + 1) * (X_SEGMENTS + 2) + x;// 下一行的顶点
				}
			}
			else
			{
				for (int x = X_SEGMENTS + 1; x != -1; --x)
				{
					m_Indices[indexIndex++] = (y + 1) * (X_SEGMENTS + 2) + x;// 下一行的顶点
					m_Indices[indexIndex++] = y * (X_SEGMENTS + 2) + x;	     // 当前行的顶点
				}
			}
			isEvenRow = !isEvenRow;
		}

		m_VertexArray = VertexArray::Create();
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(MeshVertex));
		m_VertexBuffer->SetLayout(m_BufferLayout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Indices.data(), m_IndexSize);
		m_VertexArray->SetIndexBuffer(indexBuffer);

	}

	Capsule::~Capsule()
	{
	}

	void Capsule::FlushInstances()
	{
		Renderer::DrawStripIndexInstanced(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount(), m_InstanceDataList.size());
	}

}