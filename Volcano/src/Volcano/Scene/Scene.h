#pragma once

#include "Volcano/Core/Core.h"
#include "Volcano/Core/UUID.h"
#include "Volcano/Renderer/EditorCamera.h"
#include "Volcano/Renderer/LightRenderer.h"
#include "Volcano/Scene/Components.h"

#include "entt.hpp"

class b2World;

namespace Volcano
{
	// 场景状态
	enum class SceneState
	{
		Edit = 0, Play = 1, Simulate = 2
	};

	enum class RenderType
	{
		NORMAL,
		SHADOW_DIRECTIONALLIGHT,
		SHADOW_POINTLIGHT,
		SHADOW_SPOTLIGHT,
		SHADOW,
		NORMAL_VISUALIZATION,
		G_BUFFER,
		DIRECTIONAL_LIGHT,
		POINT_LIGHT,
		SPOT_LIGHT,
		PBRLIGHT_SHADING,
		PBRDEFERRED_SHADING,
		SKYBOX,
		PARTICLE,
		OUTLINE,
		FORWARD_SHADING,
		ENTITYID,
		LIGHTING_MODE
	};
	
	struct VOL_API SSAOData
	{
		int   kernelSize = 64;
		float radius = 0.5f;
		float bias = 0.035f;
		float power = 1.0f;
		int   ssaoEnabled = 0;
		float pad[3];
	};

	class Entity;

	class VOL_API Scene
	{
	public:

		static Ref<Entity> FindEntityByName(std::string name, std::vector<Ref<Entity>>& entityList);
		static std::string NewName(std::vector<Ref<Entity>>& entityList, std::string name);
		static Ref<Scene> Copy(Ref<Scene> other);
		static Ref<Entity> Find(std::string name, std::vector<Ref<Entity>>& entityList);
		static void RemoveEntityFromList(Ref<Entity> entity, std::vector<Ref<Entity>>& entityList);
		// 将entity移动到parent下，parent为空时无效，index非法时无效
		static void MoveEntityToEntity(Ref<Entity> entity, Entity* parent, int index = -1);
		// 将entity移动到targetScene的scene路径下，targetScene为空时无效，index非法时无效
		static void MoveEntityToScene(Ref<Entity> entity, Scene* targetScene, int index = -1);
		static void TraverseEntity(std::vector<Ref<Entity>>& entityList, const std::function<bool(Ref<Entity>)>& function);

		Scene();
		~Scene();


		// 在parent下新建实体，parent为空指针则在Scene层级新建实体
		Ref<Entity> CreateEntity(const std::string& name = std::string(), Entity* parent = nullptr, int index = -1);
		
		// 在parent层级新建实体，parent为空指针则在Scene层级新建实体
		Ref<Entity> CreateEntityWithUUID(Volcano::UUID uuid, const std::string& name = std::string(), Entity* parent = nullptr, int index = -1);

		Ref<Entity> GetEntityByUUID(Volcano::UUID uuid);

		// 将entity移动到自己的scene路径下，不是index != -1 表示移动到对应位置，如果index超过list长度，则是无效移动
		void MoveEntity(Ref<Entity> entity, int index = -1);
		// 在parent下克隆entity。parent为空时，在Scene目录下克隆entity
		Ref<Entity> DuplicateEntity(Ref<Entity> entity, Entity* parent = nullptr, Volcano::UUID id = Volcano::UUID(), int index = -1);

		// 销毁entity的所有子节点
		void DestroyEntityChild(Ref<Entity> entity);
		// 销毁entity
		void DestroyEntity(Ref<Entity> entity);

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime();
		void OnRenderRuntime();
		void OnUpdateSimulation(EditorCamera& camera);
		void OnRenderSimulation();
		void OnUpdateEditor(EditorCamera& camera);
		void OnRenderEditor();

		void OnViewportResize(uint32_t width, uint32_t height);
		std::pair<uint32_t, uint32_t> GetViewportSize();

		Ref<Entity> GetPrimaryCameraEntity();

		void Step(int frames = 1);

		std::string GetName() { return m_Name; }
		void SetName(std::string name) { m_Name = name; }
		std::string GetFileRelativePath() { return m_FileRelativePath; }
		void SetFileRelativePath(std::string fileRelativePath) { m_FileRelativePath = fileRelativePath; }

		entt::registry& GetRegistry() { return m_Registry; }
		std::unordered_map<Volcano::UUID, Ref<Entity>>& GetEntityIDMap() { return m_EntityIDMap; }
		std::unordered_map<entt::entity, Ref<Entity>>& GetEntityEnttMap() { return m_EntityEnttMap; }
		std::vector<Ref<Entity>>& GetEntityList() { return m_EntityList; }
		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }
		void SetRunning(bool running) { m_IsRunning = running; }
		void SetPaused(bool paused) { m_IsPaused = paused; }
		void SetRenderType(RenderType type) { m_RenderType = type; }

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		void UpdateLight();


		MaterialLibrary* GetMaterialLibrary() { return &m_MaterialLibrary; }
		
		void SetTextureWhite(const MaterialLibraryKey& materialLibraryKey, MaterialType type)
		{
			TextureLibraryKey textureLibraryKey = { "White", Texture::GetTextureLibrary()->m_WhiteHash, true, TextureInternalFormat::NONE };
			m_MaterialLibrary.UpdateMaterial(materialLibraryKey, type, textureLibraryKey);
		}

		void SetTextureBlack(const MaterialLibraryKey& materialLibraryKey, MaterialType type)
		{
			TextureLibraryKey textureLibraryKey = { "Black", Texture::GetTextureLibrary()->m_BlackHash, true, TextureInternalFormat::NONE };
			m_MaterialLibrary.UpdateMaterial(materialLibraryKey, type, textureLibraryKey);
		}

		bool GetHDREnabled() { return m_HDREnabled; }

		void SetHDREnabled(bool enable) { m_HDREnabled = enable; }

		float GetExposure() { return m_Exposure; }

		void SetExposure(float exposure) { m_Exposure = exposure; }

		bool GetBloomEnabled() { return m_BloomEnabled; }

		void SetBloomEnabled(bool enable) { m_BloomEnabled = enable; }


		SSAOData GetSSAO() { return m_SSAO; }

		void SetSSAO(SSAOData ssao) { m_SSAO = ssao; }

	private:
		
		void UpdateScene(Camera& cameraconst, const glm::vec3& cameraWorldPosition, const glm::quat& cameraWorldRotation);
		// 渲染场景。参数：摄像头，摄像头变换矩阵，摄像头位置，摄像头方向
		void RenderScene();

		void OnPhysics();
		void OnPhysics2DStart();
		void OnPhysics2DStop();

		template<typename T>
		void OnComponentAdded(Entity& entity, T& component);

	private:
		std::string m_Name = "Untitled";
		std::string m_FileRelativePath;

		entt::registry m_Registry;                                     // 注册表，存所有entt::entity
		std::unordered_map<Volcano::UUID, Ref<Entity>> m_EntityIDMap;  // ID表，存UUID和Ref<Entity>的对应关系，包括子节点Entity
		std::unordered_map<entt::entity, Ref<Entity>> m_EntityEnttMap; // entt表，存entt::entity和Ref<Entity>的对应关系，包括子节点Entity
		std::vector<Ref<Entity>> m_EntityList;                         // entity表，存世界空间中的Entity，不包括子节点Entity

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		RenderType m_RenderType = RenderType::NORMAL;

		b2World* m_PhysicsWorld = nullptr;

		MaterialLibrary m_MaterialLibrary;

		SSAOData m_SSAO;

		bool m_HDREnabled;
		float m_Exposure;

		bool m_BloomEnabled;

		friend class Entity;
		friend class SceneRenderer;
	};

}
#include "Volcano/Scene/SceneOnComponentAdded.h"