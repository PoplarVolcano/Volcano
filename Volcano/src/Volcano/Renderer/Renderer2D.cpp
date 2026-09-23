#include "volpch.h"
#include "Volcano/Renderer/Renderer2D.h"

#include "Volcano/Renderer/Renderer.h"

#include "Volcano/Renderer/RendererItem/Circle.h"
#include "Volcano/Renderer/RendererItem/Line.h"

namespace Volcano {

	static Renderer2D::Statistics s_Statistics;

	void Renderer2D::Init()
	{
	}

	void Renderer2D::Shutdown()
	{
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		Flush();
	}

	void Renderer2D::StartBatch()
	{
		Ref<Circle> circleMesh = Mesh::GetMeshLibrary()->Get<Circle>(MeshType::Circle)->mesh;
		circleMesh->StartBatch();

		Ref<Line> lineMesh = Mesh::GetMeshLibrary()->Get<Line>(MeshType::Line)->mesh;
		lineMesh->StartBatch();


	}

	void Renderer2D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer2D::Flush()
	{
		Ref<Circle> circleMesh = Mesh::GetMeshLibrary()->Get<Circle>(MeshType::Circle)->mesh;
		circleMesh->Flush();

		Ref<Line> lineMesh = Mesh::GetMeshLibrary()->Get<Line>(MeshType::Line)->mesh;
		lineMesh->Flush();
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness, float fade, int entityID)
	{
		Ref<Circle> circleMesh = Mesh::GetMeshLibrary()->Get<Circle>(MeshType::Circle)->mesh;

		if (circleMesh->indexCount >= Circle::MaxIndices)
			circleMesh->NextBatch();

		for (size_t i = 0; i < 4; i++)
		{
			circleMesh->vertexBufferPtr->WorldPosition = transform * circleMesh->vertexPosition[i];
			circleMesh->vertexBufferPtr->LocalPosition = circleMesh->vertexPosition[i] * 2.0f; // x,y取值范围[-1, 1]，左下角(-1, -1)，右上角(1, 1)
			circleMesh->vertexBufferPtr->Color = color;
			circleMesh->vertexBufferPtr->Thickness = thickness;
			circleMesh->vertexBufferPtr->Fade = fade;
			circleMesh->vertexBufferPtr->EntityID = entityID;
			circleMesh->vertexBufferPtr++;
		}

		circleMesh->indexCount += 6;

		s_Statistics.CircleCount++;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int entityID)
	{
		Ref<Line> lineMesh = Mesh::GetMeshLibrary()->Get<Line>(MeshType::Line)->mesh;

		if (lineMesh->vertexCount >= Line::MaxVertices)
			lineMesh->NextBatch();

		lineMesh->vertexBufferPtr->Position = p0;
		lineMesh->vertexBufferPtr->Color = color;
		lineMesh->vertexBufferPtr->EntityID = entityID;
		lineMesh->vertexBufferPtr++;

		lineMesh->vertexBufferPtr->Position = p1;
		lineMesh->vertexBufferPtr->Color = color;
		lineMesh->vertexBufferPtr->EntityID = entityID;
		lineMesh->vertexBufferPtr++;

		lineMesh->vertexCount += 2;

		s_Statistics.LineCount++;
	}


	float Renderer2D::GetLineWidth()
	{
		Ref<Line> lineMesh = Mesh::GetMeshLibrary()->Get<Line>(MeshType::Line)->mesh;
		return lineMesh->width;
	}

	void Renderer2D::SetLineWidth(float width)
	{
		Ref<Line> lineMesh = Mesh::GetMeshLibrary()->Get<Line>(MeshType::Line)->mesh;
		lineMesh->width = width;
	}

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

	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		glm::vec4 position[] = {
			{ -0.5, -0.5, 0.0f, 1.0f },
			{  0.5, -0.5, 0.0f, 1.0f },
			{  0.5,  0.5, 0.0f, 1.0f },
			{ -0.5,  0.5, 0.0f, 1.0f }
		};

		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
			lineVertices[i] = transform * position[i]; // quad的顶点位置正好是rect的顶点位置

		DrawLine(lineVertices[0], lineVertices[1], color, entityID);
		DrawLine(lineVertices[1], lineVertices[2], color, entityID);
		DrawLine(lineVertices[2], lineVertices[3], color, entityID);
		DrawLine(lineVertices[3], lineVertices[0], color, entityID);
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Statistics, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Statistics;
	}

}