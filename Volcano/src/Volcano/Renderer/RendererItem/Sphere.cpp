#include "volpch.h"
#include "Sphere.h"

#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	/*
	
	X_SEGMENTS: 经度方向（环绕 Y 轴） 的分段数 ，即水平方向的扇区数
	Y_SEGMENTS: 纬度方向（从顶部到底部） 的分段数，即垂直方向的层数
	总顶点数VertexSize = (X_SEGMENTS + 1) * (Y_SEGMENTS + 1)，因为每个方向都有闭合所需的额外顶点（首尾重叠）
	总索引数IndexSize = (X_SEGMENTS + 1) * Y_SEGMENTS * 2，每个矩形格子需要 2 个三角形（6 个索引）
	
	球体用条带方式（相邻顶点共享）所以约为总顶点数的 2 倍
	对于相邻行（y 和 y+1），行数 Y_SEGMENTS + 1 有 Y_SEGMENTS 对相邻行

	glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, nullptr);
	GL_TRIANGLE_STRIP 绘制三角形条带。每相邻的 3 个顶点构成一个三角形（0,1,2；1,2,3；2,3,4...）

	数学原理（球面坐标）
	球面坐标参数化：
	第x段：xSegment = x / Segment
	经度角θ（绕 Y 轴） = xSegment * 2π，范围 [0, 2π]。
	纬度角φ（与 Y 轴的夹角） = ySegment * π，范围 [0, π]

	    y
	    |
	    |
	    |________x
       /
	  /
	z

	直角坐标转换：
	x = r * sin(θ) * cos(φ)
	y = r * cos(θ)
	z = r * sin(θ) * sin(φ)
	其中 r = 1（单位球），θ = ySegment * π，φ = xSegment * 2π
	
	法线计算
	由于球心在原点，法线就是归一化的位置向量（单位球面上任意点的法线方向指向球外）。
		
	UV 坐标
	UV = (xSegment, ySegment)，即纹理坐标在 0~1 范围内均匀映射到球面，适合使用经纬图纹理（全景图）。
	texture:
	   ___________
	 y|			  |
	  |			  |
	  |___________|
	              x


	切线（Tangent，T）：沿 经度方向（U 方向） 的切向量，即对 φ 求偏导并归一化。
	T = (-sin(φ), 0, cos(φ))
	
	双切线（Bitangent，B）：沿 纬度方向（V 方向） 的切向量，即对 θ 求偏导并归一化。
	B = (cos(φ)*cos(θ), -sin(θ), sin(φ)*cos(θ))
	

	生成三角形条带（Triangle Strip）
	“之”字形遍历
	对于 GL_TRIANGLE_STRIP，每三个连续顶点（{v0, v1, v2}, {v1, v2, v3}, ...）形成一个三角形。
	三角形 (v0, v1, v2) 的绕序由 v0 → v1 → v2 决定。
	下一个三角形 (v1, v2, v3) 会自动翻转绕序（因为顶点顺序变成 v1 → v2 → v3，与上一个三角形的方向相反）。
	0 -> 1   0    1
   /\  /	    / /\
	| /		   /  |
	2    3	 2 -> 3

	网格的每一行（行 y 固定）包含 X_SEGMENTS + 1 个顶点。为了保持三角形绕序一致，需要交替改变顶点遍历方向：
	偶数行（y = 0, 2, 4, ...）：左到右依次取顶点对 (row_y, x) 和 (row_{y+1}, x)
	奇数行（y = 1, 3, 5, ...）：从右到左取顶点对对 (row_{y+1}, x) 和 (row_y, x)

	*/
	Sphere::Sphere()
	{

		const float PI = 3.14159265359f;
		const float Radius = 0.5f;

		const uint32_t X_SEGMENTS = 64;
		const uint32_t Y_SEGMENTS = 64;
		m_VertexSize = (X_SEGMENTS + 1) * (Y_SEGMENTS + 1);
		m_IndexSize = (X_SEGMENTS + 1) * Y_SEGMENTS * 2;
		m_Vertices.resize(m_VertexSize);
		m_Indices.resize(m_IndexSize);

		for (uint32_t x = 0; x != X_SEGMENTS + 1; x++)
		{
			for (uint32_t y = 0; y != Y_SEGMENTS + 1; y++)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				//float phi = xSegment * 2.0f * PI;           // 经度角 φ，从x轴正方形为0度开始
				float phi = xSegment * 2.0f * PI - PI * 0.5f; // 经度角 φ，从z轴负方形为0度开始
				float theta = ySegment * PI;                  // 纬度角 θ
				float xDir = std::cos(phi) * std::sin(theta);
				float yDir = std::cos(theta);
				float zDir = std::sin(phi) * std::sin(theta);
				float xPos = Radius * xDir;
				float yPos = Radius * yDir;
				float zPos = Radius * zDir;

				uint32_t index = y + x * (Y_SEGMENTS + 1);
				m_Vertices[index].position = glm::vec3(xPos, yPos, zPos);
				m_Vertices[index].texCoord = glm::vec2(1.0f - xSegment, 1.0f - ySegment);
				m_Vertices[index].normal = glm::vec3(xDir, yDir, zDir);
				m_Vertices[index].tangent = glm::vec3(-std::sin(phi), 0.0f, std::cos(phi));;
				m_Vertices[index].bitangent = glm::vec3(
					std::cos(phi) * std::cos(theta),
					-sin(theta),
					sin(phi) * cos(theta));
			}
		}

		// 是否是奇数行
		// bool isOddRow = false;
		// 是否是偶数行
		bool isEvenRow = true;
		uint32_t index = 0;
		for (uint32_t y = 0; y != Y_SEGMENTS; ++y)
		{
			// 一奇一偶为一个矩形，[0,0],[0,1],[1,1],[1,0]
			if (isEvenRow) // even rows: y == 0, y == 2; and so on
			{
				for (uint32_t x = 0; x != X_SEGMENTS + 1; ++x)
				{
					m_Indices[index++] = y * (X_SEGMENTS + 1) + x;      // 当前行的顶点
					m_Indices[index++] = (y + 1) * (X_SEGMENTS + 1) + x;// 下一行的顶点
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x != -1; --x)
				{
					m_Indices[index++] = (y + 1) * (X_SEGMENTS + 1) + x;// 下一行的顶点
					m_Indices[index++] = y * (X_SEGMENTS + 1) + x;		// 当前行的顶点
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

	Sphere::~Sphere()
	{
	}

	void Sphere::FlushInstances()
	{
		Renderer::DrawStripIndexInstanced(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount(), m_InstanceDataList.size());
	}

}