#include "volpch.h"
#include "RendererModel.h"
#include "Model.h"
#include "Renderer.h"
#include "Light.h"

namespace Volcano {

	static Ref<UniformBuffer> s_LightUniformBuffer;

	static Ref<Model> s_Model;

	void RendererModel::Init()
	{
		std::string path = "Resources/Objects/nanosuit/nanosuit.obj";
		s_Model = Model::Create(path.c_str(), false);

		Renderer::GetShaderLibrary()->Load("assets/shaders/ModelLoading.glsl");
		
		// 在Renderer3D中设置过的uniform全局通用，除非有变动，否则不需要重新设置
		/*
		s_CameraUniformBuffer           = UniformBuffer::Create(4 * 4 * sizeof(float), 0);
		s_CameraPositionUniformBuffer   = UniformBuffer::Create(4 * sizeof(float), 1);
		s_DirectionalLightUniformBuffer = UniformBuffer::Create((4 + 4 + 4 + 4) * sizeof(float), 2);
		s_PointLightUniformBuffer       = UniformBuffer::Create((4 + 4 + 4 + 3 + 1 + 1 + 1) * sizeof(float), 3);
		s_SpotLightUniformBuffer        = UniformBuffer::Create((4 + 4 + 4 + 4 + 3 + 1 + 1 + 1 + 1 + 1) * sizeof(float), 4);
		s_MaterialUniformBuffer         = UniformBuffer::Create(sizeof(float), 5);
		s_LightUniformBuffer = UniformBuffer::Create((4 * 4) * sizeof(float), 6);
		*/
	}

	void RendererModel::Shutdown()
	{
	}

	void RendererModel::BeginScene(Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction)
	{
		/*
		s_CameraBuffer.viewProjection = camera.GetProjection() * glm::inverse(transform);
		s_CameraUniformBuffer->SetData(&s_CameraBuffer.viewProjection, sizeof(glm::mat4));

		// transform最后一列前三个位置为translate
		s_CameraPositionBuffer.CameraPosition = position;
		s_CameraPositionUniformBuffer->SetData(&s_CameraPositionBuffer.CameraPosition, sizeof(glm::vec3));

		s_DirectionalLightBuffer.direction = glm::vec3(-1.0f, -1.0f, -1.0f);
		s_DirectionalLightBuffer.ambient   = glm::vec3(0.05f, 0.05f, 0.05f);
		s_DirectionalLightBuffer.diffuse   = glm::vec3( 0.5f,  0.5f,  0.5f);
		s_DirectionalLightBuffer.specular  = glm::vec3( 0.5f,  0.5f,  0.5f);
		s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.direction, sizeof(glm::vec3));
		s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.ambient,   sizeof(glm::vec3), 4 * sizeof(float));
		s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.diffuse,   sizeof(glm::vec3), (4 + 4) * sizeof(float));
		s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.specular,  sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));


		s_PointLightBuffer.position  = glm::vec3( 1.0f,  1.0f,  1.0f);
		s_PointLightBuffer.ambient   = glm::vec3(0.05f, 0.05f, 0.05f);
		s_PointLightBuffer.diffuse   = glm::vec3( 0.8f,  0.8f,  0.8f);
		s_PointLightBuffer.specular  = glm::vec3( 1.0f,  1.0f,  1.0f);
		s_PointLightBuffer.constant  = 1.0f;
		s_PointLightBuffer.linear    = 0.09f;
		s_PointLightBuffer.quadratic = 0.032f;
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.position,  sizeof(glm::vec3));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.ambient,   sizeof(glm::vec3), 4 * sizeof(float));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.diffuse,   sizeof(glm::vec3), (4 + 4) * sizeof(float));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.specular,  sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.constant,  sizeof(float),     (4 + 4 + 4 + 3) * sizeof(float));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.linear,    sizeof(float),     (4 + 4 + 4 + 3 + 1) * sizeof(float));
		s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.quadratic, sizeof(float),     (4 + 4 + 4 + 3 + 1 + 1) * sizeof(float));


		s_SpotLightBuffer.position    = position;
		s_SpotLightBuffer.direction   = direction;
		s_SpotLightBuffer.ambient     = glm::vec3(0.0f, 0.0f, 0.0f);
		s_SpotLightBuffer.diffuse     = glm::vec3(1.0f, 1.0f, 1.0f);
		s_SpotLightBuffer.specular    = glm::vec3(1.0f, 1.0f, 1.0f);
		s_SpotLightBuffer.constant    = 1.0f;
		s_SpotLightBuffer.linear      = 0.09f;
		s_SpotLightBuffer.quadratic   = 0.032f;
		s_SpotLightBuffer.cutOff      = glm::cos(glm::radians(12.5f));
		s_SpotLightBuffer.outerCutOff = glm::cos(glm::radians(17.5f));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.position,    sizeof(glm::vec3));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.direction,   sizeof(glm::vec3), 4 * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.ambient,     sizeof(glm::vec3), (4 + 4) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.diffuse,     sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.specular,    sizeof(glm::vec3), (4 + 4 + 4 + 4) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.constant,    sizeof(float),     (4 + 4 + 4 + 4 + 3) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.linear,      sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.quadratic,   sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.cutOff,      sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1) * sizeof(float));
		s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.outerCutOff, sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1 + 1) * sizeof(float));

		s_MaterialBuffer.shininess = 32.0f;
		s_MaterialUniformBuffer->SetData(&s_MaterialBuffer.shininess, sizeof(float));
		s_LightUniformBuffer->SetData(&s_DirectionalLightBuffer.direction, sizeof(glm::vec3));
		s_LightUniformBuffer->SetData(&s_PointLightBuffer.position,        sizeof(glm::vec3), 4 * sizeof(float));
		s_LightUniformBuffer->SetData(&s_SpotLightBuffer.position,         sizeof(glm::vec3), (4 + 4) * sizeof(float));
		s_LightUniformBuffer->SetData(&s_SpotLightBuffer.direction,        sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
	
		*/
	}

	void RendererModel::EndScene()
	{
		Flush();
	}

	void RendererModel::Flush()
	{

	}
	void RendererModel::DrawModel(const glm::mat4& transform, const glm::mat3& normalTransform, int entityID)
	{
		Ref<Shader> shader = Renderer::GetShaderLibrary()->Get("ModelLoading");
		s_Model->Draw(*shader, transform, normalTransform, entityID);
	}

	void RendererModel::StartBatch()
	{
	}
	void RendererModel::NextBatch()
	{
	}
}