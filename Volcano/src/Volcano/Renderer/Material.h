#pragma once

#include "Texture.h"

namespace Volcano
{
	static const uint8_t s_MaterialTypeCount = 7;
	enum class MaterialType
	{
		Albedo,              // 漫反射：定义物体表面没有光照影响时的基本颜色
		Diffuse = Albedo,    // 反照率：同漫反射
		Metallic,            // 金属度：灰度图，用于定义材质表面是金属还是非金属（电介质），金属（Metallic = 1.0，白色）会强烈反射光线
		Specular = Metallic, // 高光：用于控制反射高光的强度
		Normal,              // 法线贴图：通过记录法线方向的变化，在不增加模型面数的情况下，让平面看起来有凹凸感
		Parallax,            // 高度贴图：同样用于表现凹凸感，但通过存储高度信息（灰度图）实现
		Roughness,           // 粗糙度：灰度图，定义材质表面的光滑程度，光滑表面（Roughness 接近 0.0，黑色）反射清晰，像镜子
		AO,                  // 环境光遮蔽, Ambient Occlusion：灰度图，用于模拟物体表面缝隙、角落等区域因环境光被遮挡而变暗的效果
		Emission             // 放射光贴图：定义物体表面主动发出的光，不受场景光照影响。它能精确控制哪些区域发光，常用于制作屏幕、指示灯、眼睛、熔岩等效果。
	};

	struct Material
	{
		TextureLibraryKey diffuse;  // 漫反射
		TextureLibraryKey specular; // 镜面反射
		TextureLibraryKey normal;   // 法线
		TextureLibraryKey parallax; // 视差
		TextureLibraryKey roughness;// 粗糙度
		TextureLibraryKey ao;	    // 环境光遮蔽
		TextureLibraryKey emission; // 放射光

		int lightingMode = 0; // 0: Unlit, 1: BlinnPhong, 2: PBR
	};

	// 材质纹理句柄（handle）
	struct MaterialHandle
	{
		uint64_t diffuse;  // 漫反射
		uint64_t specular; // 镜面反射
		uint64_t normal;   // 法线
		uint64_t parallax; // 视差
		uint64_t roughness;// 粗糙度
		uint64_t ao;	   // 环境光遮蔽
		uint64_t emission; // 放射光
	};


	struct VOL_API MaterialLibraryKey
	{
		std::string key;
		uint64_t keyHash;

		bool operator==(const MaterialLibraryKey& other) const
		{
			return key == other.key;
		}
	};

	// map进行查找时调用operator()，如果没有哈希冲突的话就命中，如果有哈希冲突的话调用MaterialLibraryKey的operator==
	struct MaterialLibraryKeyHash
	{
		size_t operator()(const MaterialLibraryKey& k) const
		{
			return k.keyHash;
		}
	};

	// 维护一个材质数组和一个材质句柄数组
	// 通过材质名和数组索引的互相检索对两个数组进行增删改查
	// 使用FreeSlots作为目前可用空数组位的索引的栈，删除材质时将索引入栈，新增材质时将索引出栈
	class VOL_API MaterialLibrary
	{
	public:
		MaterialLibrary();
		~MaterialLibrary();

		MaterialLibraryKey AddMaterial(const MaterialLibraryKey& materialLibraryKey, uint32_t index, Material material);
		MaterialLibraryKey AddMaterial(const MaterialLibraryKey& materialLibraryKey, Material material);
		MaterialLibraryKey AddMaterial(Material material);
		MaterialLibraryKey AddMaterial();
		void RemoveMaterial(const MaterialLibraryKey& materialLibraryKey, MaterialType type);
		void RemoveMaterial(const MaterialLibraryKey& materialLibraryKey);
		void UpdateMaterialNamesCStr();

		const Material* GetMaterial(const MaterialLibraryKey& materialLibraryKey);
		const std::vector<Material>& GetMaterials() { return m_Materials; }
		const MaterialHandle* GetMaterialHandle(const MaterialLibraryKey& materialLibraryKey);
		const std::vector<MaterialHandle>& GetMaterialHandles() { return m_MaterialHandles; }
		const std::unordered_map<MaterialLibraryKey, uint32_t, MaterialLibraryKeyHash>& GetMaterialNameMap() { return m_MaterialNameToIndexMap; }
		const std::vector<const char*>& GetMaterialNamesCStr() { return m_MaterialNamesCStr; }

		void UpdateMaterial(const MaterialLibraryKey& materialLibraryKey, MaterialType type, TextureLibraryKey& textureLibraryKey);
		void UpdateMaterial(const MaterialLibraryKey& materialLibraryKey, int lightingMode);
		void UpdateMaterial(const MaterialLibraryKey& materialLibraryKey, Material material);
		void UpdateMaterial(uint32_t index, MaterialType type, TextureLibraryKey& textureLibraryKey);
		void UpdateMaterial(uint32_t index, int lightingMode);
		void UpdateMaterial(uint32_t index, Material material);
		void UpdateMaterialName(const MaterialLibraryKey& oldKey, const MaterialLibraryKey& newKey);

		MaterialLibraryKey* GetKey(uint32_t index);
		uint32_t GetIndex(const MaterialLibraryKey& materialLibraryKey);

		void Copy(const MaterialLibrary& materialLibrary);

	private:
		static MaterialHandle MaterialToMaterialHandle(Material& material);
		static bool IsMaterialValid(const Material& material);

	private:
		std::vector<Material> m_Materials;
		std::vector<MaterialHandle> m_MaterialHandles;
		std::unordered_map<MaterialLibraryKey, uint32_t, MaterialLibraryKeyHash> m_MaterialNameToIndexMap;
		std::unordered_map<uint32_t, MaterialLibraryKey> m_MaterialIndexToNameMap;
		std::vector<const char*> m_MaterialNamesCStr;
		std::vector<uint32_t> m_FreeSlots;
	};
}