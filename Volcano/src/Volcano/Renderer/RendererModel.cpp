#include "volpch.h"
#include "RendererModel.h"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/RendererItem/Model.h"
#include "Volcano/Renderer/RendererItem/Animation.h"
#include "Volcano/Renderer/RendererItem/Animator.h"
#include "Volcano/Utils/PlatformUtils.h"

#include "Light.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <glad/glad.h>

namespace Volcano {

	static Ref<UniformBuffer> s_LightUniformBuffer;
	static std::unordered_map<std::string, Ref<Model>> s_ModelMap;

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
	static Ref<Animation> s_Animation;
	static Ref<Animator> s_Animator;

#define MAX_BONES = 100;
	static Ref<UniformBuffer> s_BonesMatricesUniformBuffer;


	void RendererModel::Init()
	{
		//std::string path = "SandBoxProject/Assets/Objects/nanosuit/nanosuit.obj";
		std::string path = "SandBoxProject/Assets/Objects/vampire/dancing_vampire.dae";
		//std::string path = "SandBoxProject/Assets/Objects/m1911/M1911Materials.fbx";
		//std::string path = "SandBoxProject/Assets/Objects/cyborg/cyborg.obj";
		s_ModelMap["nanosuit"] = Model::Create(path.c_str(), false);

		s_Animation = std::make_shared<Animation>(std::string("SandBoxProject/Assets/Objects/vampire/dancing_vampire.dae"), s_ModelMap["nanosuit"].get());
		s_Animator = std::make_shared<Animator>(s_Animation.get());

		Renderer::GetShaderLibrary()->Load("assets/shaders/ModelLoading.glsl");
		
		// 在Renderer3D中设置过的uniform全局通用，除非有变动，否则不需要重新设置
		s_TransformUniformBuffer = UniformBuffer::Create(4 * 4 * 2 * sizeof(float), 6);

		s_BonesMatricesUniformBuffer = UniformBuffer::Create(100 * 4 * 4 * sizeof(float), 15);
	}

	void RendererModel::Shutdown()
	{
	}

	void RendererModel::Update(Timestep ts)
	{
		s_Animator->UpdateAnimation(ts.GetSeconds());
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
		auto finalBoneMatrices = s_Animator->GetFinalBoneMatrices();
		s_BonesMatricesUniformBuffer->SetData(&finalBoneMatrices, finalBoneMatrices.size() * 4 * 4 * sizeof(float));

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
			Renderer::GetShaderLibrary()->Get("ModelLoading")->Bind();
			break;
		default:
			VOL_CORE_ASSERT(0);
		}
		s_ModelMap["nanosuit"]->DrawIndexed();

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
		std::vector<glm::mat4> finalBoneMatrices = s_Animator->GetFinalBoneMatrices();
		s_ModelMap["nanosuit"]->Draw(*shader, transform, normalTransform, entityID, finalBoneMatrices);

	}

	void RendererModel::StartBatch()
	{
	}
	void RendererModel::NextBatch()
	{
	}
}