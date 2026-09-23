#include "volpch.h"
#include "Cone.h"

#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	Cone::Cone()
	{
		const uint32_t segments = 32;

		m_VertexSize = 2 + segments;
		m_IndexSize = segments * 6;
		m_Vertices.reserve(m_VertexSize);
		m_Indices.reserve(m_IndexSize);

		// 锥尖在 +Z，底面圆心在原点
		// 锥尖（索引0）
		m_Vertices.emplace_back(
			glm::vec3(0.0f, 0.0f, 1.0f),   // 位置
			glm::vec2(0.5f, 1.0f),         // 纹理坐标
			glm::vec3(0.0f, 0.0f, 1.0f),   // 法线（朝 Z）
			glm::vec3(1.0f, 0.0f, 0.0f),   // 切线
			glm::vec3(0.0f, 1.0f, 0.0f)    // 副切线
		);

		// 底面圆心（索引 1）
		m_Vertices.emplace_back(
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec2(0.5f, 1.0f),
			glm::vec3(0.0f, 0.0f, -1.0f),  // 法线朝 -Z
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		float reciprocalSegments = 1.0f / (float)segments;
		// 底面圆周顶点（索引 2 ~ segments+1）
		for (uint32_t i = 0; i != segments; i++)
		{
			float angle = (float)i * reciprocalSegments * 2.0f * 3.1415926535f;
			float x = cos(angle);
			float y = sin(angle);

			// 侧面法线：指向径向方向（x, y, 0）
			glm::vec3 normal(x, y, 0.0f);
			normal = glm::normalize(normal);

			// UV：U 按角度展开（0~1），V 为 0（底部）
			float u = (float)i * reciprocalSegments;
			glm::vec2 uv(u, 0.0f);

			m_Vertices.emplace_back(
				glm::vec3(x, y, 0.0f),
				uv,
				normal,
				glm::vec3(1.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f)
			);
		}

		// 索引
		// 侧面三角形：锥尖 -> 圆周顶点 i -> 圆周顶点 i+1
		for (uint32_t i = 0; i != segments; i++)
		{
			uint32_t current = i + 2;
			uint32_t next = (i + 1) % segments + 2;
			m_Indices.push_back(0);    // 锥尖
			m_Indices.push_back(current);
			m_Indices.push_back(next);
		}

		// 底面三角形：圆心 -> 圆周顶点 i -> 圆周顶点 i+1
		for (uint32_t i = 0; i != segments; i++)
		{
			uint32_t curr = i + 2;
			uint32_t next = (i + 1) % segments + 2;
			m_Indices.push_back(1);    // 底面圆心
			m_Indices.push_back(next);
			m_Indices.push_back(curr);
		}

		m_VertexArray = VertexArray::Create();
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(MeshVertex));
		m_VertexBuffer->SetLayout(m_BufferLayout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Indices.data(), m_IndexSize);
		m_VertexArray->SetIndexBuffer(indexBuffer);

	}

	Cone::~Cone()
	{
	}

	void Cone::FlushInstances()
	{
		Renderer::DrawIndexInstanced(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount(), m_InstanceDataList.size());
	}


}