#include "volpch.h"
#include "RendererModel.h"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/RendererItem/Model.h"
#include "Volcano/Utils/PlatformUtils.h"

#include "Light.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <glad/glad.h>

namespace Volcano {

	static Ref<UniformBuffer> s_LightUniformBuffer;
	static std::unordered_map<std::string, Ref<Model>> s_Models;

	struct RockData
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		glm::mat4 ModelMatrice;
	};


	struct TransformBuffer
	{
		glm::mat4 Transform;
		glm::mat3 NormalTransform;
	};
	static TransformBuffer s_TransformBuffer;
	static Ref<UniformBuffer> s_TransformUniformBuffer;

	static Ref<Model> s_Model;

	void RendererModel::Init()
	{
		std::string path = "SandBoxProject/Assets/Objects/nanosuit/nanosuit.obj";
		s_Model = Model::Create(path.c_str(), false);

		Renderer::GetShaderLibrary()->Load("assets/shaders/ModelLoading.glsl");
		
		// 在Renderer3D中设置过的uniform全局通用，除非有变动，否则不需要重新设置
		s_TransformUniformBuffer = UniformBuffer::Create(4 * 4 * 2 * sizeof(float), 6);


	}

	void RendererModel::Shutdown()
	{
	}

	void RendererModel::BeginScene(Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction)
	{
	}

	void RendererModel::EndScene(RenderType type)
	{
		Flush(type);
	}

	void RendererModel::Flush(RenderType type)
	{
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
		case RenderType::NORMAL:
			Renderer::GetShaderLibrary()->Get("ModelLoading")->Bind();
			break;
		default:
			VOL_CORE_ASSERT(0);
		}
		s_Model->DrawIndexed();

	}

	void RendererModel::DrawModel(const glm::mat4& transform, const glm::mat3& normalTransform, std::string& modelPath, int entityID)
	{
		// 路径为空，跳过
		if (modelPath.empty())
			return;
		/*
		if (s_Models.find(modelPath) == s_Models.end())
		{
			Ref<Model> model = Model::Create(modelPath.c_str(), false);
			// path成功读取到model，将model加入map
			if (model)
				s_Models.emplace(modelPath, model);
		}
		*/
		//if (s_Models.find(modelPath) == s_Models.end())
		//	return;

		//Ref<Model> model = s_Models.at(modelPath);

		s_TransformBuffer.Transform = transform;
		s_TransformBuffer.NormalTransform = normalTransform;
		s_TransformUniformBuffer->SetData(&s_TransformBuffer.Transform,       sizeof(glm::mat4));
		s_TransformUniformBuffer->SetData(&s_TransformBuffer.NormalTransform, sizeof(glm::mat4), 4 * 4 * sizeof(float));
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