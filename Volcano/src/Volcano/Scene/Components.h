#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Volcano/Scene/SceneCamera.h"
#include "Volcano/Renderer/Texture.h"
#include "Volcano/Core/UUID.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "Volcano/Renderer/RendererItem/Mesh.h"
#include "Volcano/Renderer/RendererItem/CubeMesh.h"
#include "Volcano/Renderer/RendererItem/SphereMesh.h"
#include "Volcano/Renderer/RendererItem/ModelMesh.h"
#include "Volcano/Renderer/RendererItem/Animator.h"

namespace Volcano {

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const UUID& uuid)
			: ID(uuid) {}
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	// 空组件会被视为一个空结构，被视为特殊情况，并且它不会编译
	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f }; // 弧度！！！
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}
		glm::mat3 GetNormalTransform() const
		{
			return  glm::mat3(transpose(inverse(GetTransform())));
		}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4 color)
			: Color(color) {}
	};

	struct MeshComponent
	{
		struct VertexBone
		{
			int vertexIndex1 = -1;
			int vertexIndex2 = -1;
			int boneIndex = -1;
			float weight = 0;
		};

		// 对于model中读取的mesh，需要在序列化的时候同时保存model的路径和model中的索引以读取mesh信息
		MeshType meshType = MeshType::None;
		Ref<Mesh> mesh;
		std::string modelPath = std::string();
		std::vector<VertexBone> vertexBone;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		void SetMesh(MeshType type, Entity* entity, Ref<Mesh> modelMesh = nullptr) {
			meshType = type;
			modelPath = std::string();
			vertexBone.clear();
			switch (type)
			{
			case MeshType::None:
				mesh = nullptr;
				break;
			case MeshType::Cube:
				mesh = std::make_shared<CubeMesh>();
				mesh->SetEntity(entity);
				break;
			case MeshType::Sphere:
				mesh = std::make_shared<SphereMesh>();
				mesh->SetEntity(entity);
				break;
			case MeshType::Model:
				if (modelMesh != nullptr)
				{
					mesh = std::make_shared<Mesh>(*modelMesh.get());
					mesh->ResetVertexBufferBase();
					mesh->SetEntity(entity);
				}
				else
					mesh = nullptr;
				break;
			default:
				VOL_TRACE("MeshComponent: 无效MeshType");
				mesh = nullptr;
				break;
			}
		}
	};

	struct MeshRendererComponent
	{
		std::vector<std::pair<ImageType, Ref<Texture>>> Textures;
		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent&) = default;
		void SetTexture(ImageType type, Ref<Texture> texture, uint32_t index) { Textures[index] = { type, texture }; }
		void DeleteTexture(uint32_t index) { Textures.erase(Textures.begin() + index); }
		void AddTexture(ImageType type, Ref<Texture> texture) { Textures.push_back({ type, texture }); }
		void AddTexture(ImageType type = ImageType::Diffuse)
		{
			if (type == ImageType::Diffuse)
			{
				Ref<Texture2D> whiteTexture = Texture2D::Create(1, 1);
				uint32_t whiteTextureData = 0xffffffff;
				whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
				Textures.push_back({ type, whiteTexture });
			}
			else
			{
				Ref<Texture2D> blackTexture = Texture2D::Create(1, 1);
				uint32_t blackTextureData = 0x00000000;
				blackTexture->SetData(&blackTextureData, sizeof(uint32_t));
				Textures.push_back({ type, blackTexture });
			}

		}
		void SetTextureWhite(uint32_t index) 
		{
			Ref<Texture2D> whiteTexture = Texture2D::Create(1, 1);
			uint32_t whiteTextureData = 0xffffffff;
			whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
			Textures[index].second = whiteTexture;
		}
		void SetTextureBlack(uint32_t index)
		{
			Ref<Texture2D> blackTexture = Texture2D::Create(1, 1);
			uint32_t blackTextureData = 0x00000000;
			blackTexture->SetData(&blackTextureData, sizeof(uint32_t));
			Textures[index].second = blackTexture;
		}
	};

	struct CircleRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct AnimatorComponent
	{
		Ref<Animator> animator = std::make_shared<Animator>();

		AnimatorComponent() = default;
		AnimatorComponent(const AnimatorComponent&) = default;
	};

	struct AnimationComponent
	{
		Ref<Animation> animation = std::make_shared<Animation>();
		int key;  //当前关键帧
		
		int boneIDBuffer;
		std::string boneNameBuffer;
		AssimpNodeData* newBoneNodeParent;


		AnimationComponent() = default;
		AnimationComponent(const AnimationComponent&) = default;
		void LoadAnimation(std::string path, Model* model = nullptr)
		{
			animation = std::make_shared<Animation>(path, model);
		}
	};

	struct LightComponent
	{
		enum class LightType { DirectionalLight, PointLight, SpotLight };

		LightType Type        = LightType::DirectionalLight;
		glm::vec3 Ambient     = glm::vec3(0.1f);
		glm::vec3 Diffuse 	  = glm::vec3(0.5f);
		glm::vec3 Specular 	  = glm::vec3(0.5f);
		float     Constant 	  = 1.0f;
		float     Linear 	  = 0.09f;
		float     Quadratic   = 0.032f;
		float     CutOff 	  = glm::cos(glm::radians(12.5f));
		float     OuterCutOff = glm::cos(glm::radians(17.5f));

		LightComponent() = default;
		LightComponent(const LightComponent&) = default;
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	// Forward declaration
	class ScriptableEntity;


	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent *);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		};

	};

	// Physics

	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		// 运行时候物体的物理对象
		// Storage for runtime
		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

		// TODO:移到物理材质
		// 密度,0是静态的物理
		float Density = 1.0f;
		// 摩擦力
		float Friction = 0.5f;
		// 弹力，0不会弹跳，1无限弹跳
		float Restitution = 0.5f;
		// 复原速度阈值，超过这个速度的碰撞就会被恢复原状（会反弹）。
		float RestitutionThreshold = 0.5f;

		// 运行时候由于物理，每一帧的上述参数可能会变，所以保存为对象,但未使用
		// Storage for runtime
		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

	template<typename... Component>
	struct ComponentGroup
	{
	};

	// (except IDComponent and TagComponent)
	using AllComponents = ComponentGroup<
								TransformComponent, 
								SpriteRendererComponent,
		                        MeshComponent,
		                        MeshRendererComponent,
								CircleRendererComponent,
		                        AnimatorComponent,
		                        AnimationComponent,
								LightComponent,
								CameraComponent, 
								ScriptComponent,
								NativeScriptComponent,
								Rigidbody2DComponent, 
								BoxCollider2DComponent, 
								CircleCollider2DComponent>;

}