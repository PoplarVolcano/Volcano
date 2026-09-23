#include "volpch.h"

#include "Particle.h"
#include "ParticleSystem.h" 

#include "Volcano/Core/Time.h"
#include "Volcano/Scene/Entity.h"
#include "Volcano/Renderer/RendererItem/Quad.h"
#include "Volcano/Renderer/RendererItem/Plane.h"
#include "Volcano/Renderer/RendererItem/Cube.h"
#include "Volcano/Renderer/RendererItem/Sphere.h"
#include "Volcano/Renderer/RendererItem/Cylinder.h"
#include "Volcano/Renderer/RendererItem/Capsule.h"
#include "Volcano/Renderer/RendererItem/Cone.h"

namespace Volcano {

	void Particle::Update()
	{
		float deltaTime = Time::GetDeltaTime();
		if (particleSystem->isPlaying)
		{
			position += velocity * deltaTime;
			if (!particleSystem->rotationOverLifetime.separateAxes)
			{
				rotation = glm::angleAxis(angularVelocity * deltaTime, axisOfRotation) * rotation;
			}
			else
			{
				rotation = glm::quat(angularVelocity3D * deltaTime) * rotation;
			}
		}
	}

	void Particle::Render()
	{
		glm::vec3 worldPosition = position;
		glm::mat4 worldRotation = glm::toMat4(rotation);
		if (particleSystem->simulationSpace == 0)
		{
			worldPosition = particleSystem->entity->GetWorldTransform() * glm::vec4(worldPosition, 1.0f);
			worldRotation = glm::toMat4(particleSystem->entity->GetWorldRotation()) * worldRotation;
		}
		glm::mat4 worldTransform(1.0f);
		glm::mat4 normalTransform(1.0f);

		switch (particleSystem->renderer.renderMode)
		{
		case ParticleSystem_Renderer::RenderMode::Billboard:
		{
			// 粒子面向摄像机（Billboard）
			// 摄像机自身局部坐标系的 Y 轴在世界空间中的方向
			glm::vec3 up = glm::quat(particleSystem->cameraRotation) * glm::vec3(0.0f, 1.0f, 0.0f);

			// glm::lookAt用于构建观察矩阵（View Matrix），将世界空间中的坐标转换到摄像机空间（View Space）
			// eye：摄像机在世界空间中的位置
			// center：摄像机注视的目标点（世界坐标）
			// up：世界空间中的“上方向”参考向量（通常为 glm::vec3(0, 1, 0)）
			// 这里构造一个"虚拟相机"，该相机位于原点，看向"粒子相对摄像机的偏移方向"，并使用摄像机的上方向作为参考
			// 实际上是一个 "定向旋转矩阵"：将世界坐标系的向量旋转到"从摄像机看向粒子的方向"
			glm::mat4 lookAt = glm::lookAt(glm::vec3(0.0f), worldPosition - particleSystem->cameraPosition, up);

			// lookAt 矩阵没有缩放就是一个正交矩阵，逆矩阵 = 转置矩阵
			worldTransform =
				glm::translate(glm::mat4(1.0f), worldPosition)
				* glm::transpose(lookAt)
				* glm::toMat4(glm::quat(worldRotation))
				* glm::scale(glm::mat4(1.0f), startSize);
			normalTransform = transpose(inverse(glm::mat3(worldTransform)));
			break;
		}
		default:
		{
			worldTransform =
				glm::translate(glm::mat4(1.0f), worldPosition)
				* glm::toMat4(glm::quat(worldRotation))
				* glm::scale(glm::mat4(1.0f), startSize);
			normalTransform = transpose(inverse(glm::mat3(worldTransform)));
			break;
		}
		}

		InstanceDataTotal instanceDataTotal;
		auto& materialLibraryKey = particleSystem->renderer.materialLibraryKey;
		uint32_t materialIndex = particleSystem->entity->GetScene()->GetMaterialLibrary()->GetIndex(materialLibraryKey);

		instanceDataTotal.instanceData = { worldTransform };

		instanceDataTotal.material = {
			normalTransform,
			startColor,
			{ 0.0f, 0.0f, 1.0f, 1.0f },
			0.0f,
			1.0f,
			materialIndex
		};

		instanceDataTotal.entityID.entityID = (int)particleSystem->entity->GetEntityHandle();
		instanceDataTotal.lightingMode.lightingMode = (int)particleSystem->entity->GetScene()->GetMaterialLibrary()->GetMaterial(materialLibraryKey)->lightingMode;

		switch ((MeshType)meshType)
		{
		case MeshType::None:
			break;
		case MeshType::Quad:
		{
			auto quadMesh = Mesh::GetMeshLibrary()->Get<Quad>(MeshType::Quad)->mesh;
			quadMesh->AddInstance(instanceDataTotal);
			break;
		}
		case MeshType::Circle:
			//Renderer2D::DrawCircle(instanceDataTotal.instanceData.transform, instanceDataTotal.instanceData.color, 1.0f, 0.2f, instanceDataTotal.instanceData.entityID);
			break;
		case MeshType::Line:
			break;
		case MeshType::Plane:
		{
			auto planeMesh = Mesh::GetMeshLibrary()->Get<Plane>(MeshType::Plane)->mesh;
			planeMesh->AddInstance(instanceDataTotal);
			break;
		}
		case MeshType::Cube:
		{
			auto cubeMesh = Mesh::GetMeshLibrary()->Get<Cube>(MeshType::Cube)->mesh;
			cubeMesh->AddInstance(instanceDataTotal);
			break;
		}
		case MeshType::Sphere:
		{
			auto sphereMesh = Mesh::GetMeshLibrary()->Get<Sphere>(MeshType::Sphere)->mesh;
			sphereMesh->AddInstance(instanceDataTotal);
			break;
		}
		case MeshType::Cylinder:
		{
			auto cylinderMesh = Mesh::GetMeshLibrary()->Get<Cylinder>(MeshType::Cylinder)->mesh;
			cylinderMesh->AddInstance(instanceDataTotal);
			break;
		}
		case MeshType::Capsule:
		{
			auto capsuleMesh = Mesh::GetMeshLibrary()->Get<Capsule>(MeshType::Capsule)->mesh;
			capsuleMesh->AddInstance(instanceDataTotal);
			break;
		}
		case MeshType::Cone:
		{
			auto coneMesh = Mesh::GetMeshLibrary()->Get<Cone>(MeshType::Cone)->mesh;
			coneMesh->AddInstance(instanceDataTotal);
			break;
		}
		case MeshType::Model:
			break;
		default:
			VOL_TRACE("错误MeshType");
			break;
		}
	}
}