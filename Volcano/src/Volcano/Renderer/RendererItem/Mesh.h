#pragma once

#include "Volcano/Core/Core.h"
#include "Volcano/Renderer/Texture.h"
#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Camera.h"
#include "Volcano/Renderer/InstanceData.h"

#include "glm/glm.hpp"

namespace Volcano {
	
	enum class MeshType
	{
		None,    // 无网格
		Quad,	 // 矩形
		Circle,	 // 圆
		Line,	 // 线
		Plane,	 // 平面
		Cube,	 // 方块
		Sphere,	 // 球
		Cylinder,// 圆柱体
		Capsule, // 胶囊
		Cone,    // 锥体
		Model,	 // 模型
		Skybox
	};

	class Mesh;
	class MeshLibrary;

	struct VOL_API MeshNodeBase
	{
		virtual ~MeshNodeBase() = default;
		std::string name;
	};

	template<typename TMesh>
	struct VOL_API MeshNode : public MeshNodeBase
	{
		Ref<TMesh> mesh;
	};

	struct MeshVertex
	{
		glm::vec3 position;
		glm::vec2 texCoord;
		glm::vec3 normal;   // 法线
		glm::vec3 tangent;	// 切线
		glm::vec3 bitangent;// 双切线
	};

	class Mesh
	{
	public:
		static void Init();
		static const Scope<MeshLibrary>& GetMeshLibrary();
		static Ref<Texture2D> GetWhiteTexture();
		static Ref<Texture2D> GetBlackTexture();
		static VOL_API Ref<TextureCube> GetWhiteTextureCube();
		static VOL_API Ref<TextureCube> GetBlackTextureCube();
		static void BeginScene();
		static void EndScene();

		void StartBatch();
		void NextBatch();
		void Flush();

		void ClearInstance();
		void AddInstance(InstanceDataTotal instanceDataTotal);

		uint32_t GetInstanceCount() { return m_InstanceDataList.size(); }

		virtual void FlushInstances() = 0;
	protected:

		Mesh();

	protected:

		uint32_t m_VertexSize;
		uint32_t m_IndexSize;

		std::vector<MeshVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		BufferLayout m_BufferLayout;
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;

		std::vector<InstanceData> m_InstanceDataList;
		std::vector<InstanceDataMaterial> m_InstanceDataMaterialList;
		std::vector<InstanceDataExplosion> m_InstanceDataExplosionList;
		std::vector<InstanceDataOutline> m_InstanceDataOutlineList;
		std::vector<InstanceDataNormalVisualization> m_InstanceDataNormalVisualizationList;
		std::vector<InstanceDataEntityID> m_InstanceDataEntityIDList;
		std::vector<InstanceDataLightingMode> m_InstanceDataLightingModeList;

		static std::once_flag m_MeshLibraryInitFlag;
		static Scope<MeshLibrary> m_MeshLibrary;
	};

	// 网格库，存放已声明的网格种类
	class VOL_API MeshLibrary
	{
	public:
		static MeshType StringToMeshType(const std::string& str);
		static std::string MeshTypeToString(MeshType meshType);

		MeshLibrary() = default;
		~MeshLibrary() = default;

		template<typename TMesh>
		void Add(const Ref<MeshNode<TMesh>>& meshNode);

		template<typename TMesh>
		void AddWithName(const std::string& key, const Ref<MeshNode<TMesh>>& meshNode);

		template<typename TMesh>
		Ref<MeshNode<TMesh>> Load(const std::string key);

		template<typename TMesh>
		Ref<MeshNode<TMesh>> Get(const std::string& key)
		{
			auto it = m_Meshes.find(key);
			if (it == m_Meshes.end())
			{
				Add(key);
				it = m_Meshes.find(key);
				if (it == m_Meshes.end())
				{
					VOL_CORE_ASSERT("ModelLibrary::Get：未找到Mesh");
					return nullptr;
				}
			}
			return std::dynamic_pointer_cast<MeshNode<TMesh>>(it->second);
		}

		template<typename TMesh>
		Ref<MeshNode<TMesh>> Get(MeshType meshType)
		{
			return Get<TMesh>(MeshTypeToString(meshType));
		}

		void Add(const std::string& key);

		bool Exists(const std::string& key);
		void Remove(const std::string& key);
		auto& GetMeshes() { return m_Meshes; }

	private: 
		// 禁止拷贝
		MeshLibrary(const MeshLibrary&) = delete;
		MeshLibrary& operator=(const MeshLibrary&) = delete;

		std::unordered_map<std::string, Ref<MeshNodeBase>> m_Meshes;
	};
}