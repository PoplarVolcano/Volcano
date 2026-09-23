#include "volpch.h"
#include "Material.h"

#include "Volcano/Core/Core.h"

namespace Volcano
{
	MaterialLibrary::MaterialLibrary()
	{
		Material material = {
			{ "White", Texture::GetTextureLibrary()->m_WhiteHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true }
		};
		MaterialHandle materialHandle = MaterialToMaterialHandle(material);

		m_Materials.push_back(std::move(material));
		m_MaterialHandles.push_back(materialHandle);

		std::string name = "Default";
		MaterialLibraryKey key = { name, std::hash<std::string>{}(name) };
		m_MaterialNameToIndexMap[key] = m_Materials.size() - 1;
		m_MaterialIndexToNameMap[m_Materials.size() - 1] = key;
		UpdateMaterialNamesCStr();
	}

	MaterialLibrary::~MaterialLibrary()
	{
	}

	MaterialLibraryKey MaterialLibrary::AddMaterial(const MaterialLibraryKey& materialLibraryKey, uint32_t index, Material material)
	{
		// 检查目标材质名是否存在，存在则结束添加
		if (m_MaterialNameToIndexMap.find(materialLibraryKey) != m_MaterialNameToIndexMap.end())
			return materialLibraryKey;

		// 检查index是否超出现有数组长度
		if (index >= m_Materials.size())
		{
			uint32_t sizeBefore = m_Materials.size();
			m_Materials.resize(index + 1);
			m_MaterialHandles.resize(index + 1);
			for (uint32_t i = sizeBefore; i != index + 1; i++)
			{
				m_FreeSlots.push_back(i);
			}
		}
		else
		{
			auto it = std::find(m_FreeSlots.begin(), m_FreeSlots.end(), index);
			if (it != m_FreeSlots.end())
			{
				std::swap(*it, m_FreeSlots.back());
			}
		}
		auto key = AddMaterial(materialLibraryKey, material);
		return materialLibraryKey;

	}

	MaterialLibraryKey MaterialLibrary::AddMaterial(const MaterialLibraryKey& materialLibraryKey, Material material)
	{
		if (!IsMaterialValid(material))
			return *GetKey(0);

		MaterialHandle materialHandle = MaterialToMaterialHandle(material);

		if (!m_FreeSlots.empty())
		{
			uint32_t index = m_FreeSlots.back();
			m_FreeSlots.pop_back();
			m_Materials[index] = std::move(material);
			m_MaterialHandles[index] = materialHandle;
			m_MaterialNameToIndexMap[materialLibraryKey] = index;
			m_MaterialIndexToNameMap[index] = materialLibraryKey;
			UpdateMaterialNamesCStr();
			return materialLibraryKey;
		}
		m_Materials.push_back(std::move(material));
		m_MaterialHandles.push_back(materialHandle);
		m_MaterialNameToIndexMap[materialLibraryKey] = m_Materials.size() - 1;
		m_MaterialIndexToNameMap[m_Materials.size() - 1] = materialLibraryKey;
		UpdateMaterialNamesCStr();
		return materialLibraryKey;
	}

	MaterialLibraryKey MaterialLibrary::AddMaterial(Material material)
	{
		if (!IsMaterialValid(material))
			return *GetKey(0);

		MaterialHandle materialHandle = MaterialToMaterialHandle(material);

		std::string name = "Untitled";
		std::string newName = name;
		int i = 0;
		while ([this, newName]()->bool {
			for (auto& [key, index] : m_MaterialNameToIndexMap)
			{
				if (key.key == newName)
				{
					return true;
				}
			}
			return false;
			}())
		{
			newName = name + "(" + std::to_string(i) + ")";
			i++;
		}

		if (!m_FreeSlots.empty())
		{
			uint32_t index = m_FreeSlots.back();
			m_FreeSlots.pop_back();
			m_Materials[index] = std::move(material);
			m_MaterialHandles[index] = materialHandle;
			MaterialLibraryKey key = { newName, std::hash<std::string>{}(newName) };
			m_MaterialNameToIndexMap[key] = index;
			m_MaterialIndexToNameMap[index] = key;
			UpdateMaterialNamesCStr();
			return key;
		}
		m_Materials.push_back(std::move(material));
		m_MaterialHandles.push_back(materialHandle);
		MaterialLibraryKey key = { newName, std::hash<std::string>{}(newName) };
		m_MaterialNameToIndexMap[key] = m_Materials.size() - 1;
		m_MaterialIndexToNameMap[m_Materials.size() - 1] = key;
		UpdateMaterialNamesCStr();
		return key;
	}

	MaterialLibraryKey MaterialLibrary::AddMaterial()
	{
		Material material = {
			{ "White", Texture::GetTextureLibrary()->m_WhiteHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true },
			{ "Black", Texture::GetTextureLibrary()->m_BlackHash, true }
		};
		return AddMaterial(material);
	}

	void MaterialLibrary::RemoveMaterial(const MaterialLibraryKey& materialLibraryKey, MaterialType type)
	{
		if (materialLibraryKey.key == "Default")
			return;

		if (type == MaterialType::Diffuse)
		{
			TextureLibraryKey textureLibraryKey = { "White", Texture::GetTextureLibrary()->m_WhiteHash, true, TextureInternalFormat::NONE };
			UpdateMaterial(materialLibraryKey, type, textureLibraryKey);
		}
		else
		{
			TextureLibraryKey textureLibraryKey = { "Black", Texture::GetTextureLibrary()->m_BlackHash, true, TextureInternalFormat::NONE };
			UpdateMaterial(materialLibraryKey, type, textureLibraryKey);
		}
	}

	void MaterialLibrary::RemoveMaterial(const MaterialLibraryKey& materialLibraryKey)
	{
		if (materialLibraryKey.key == "Default")
			return;

		auto it1 = m_MaterialNameToIndexMap.find(materialLibraryKey);
		if (it1 == m_MaterialNameToIndexMap.end())
			return;

		uint32_t index = it1->second;
		m_Materials[index] = {};
		m_MaterialHandles[index] = {};
		m_MaterialNameToIndexMap.erase(it1);

		auto it2 = m_MaterialIndexToNameMap.find(index);
		if (it2 != m_MaterialIndexToNameMap.end())
		{
			m_MaterialIndexToNameMap.erase(it2);
		}

		UpdateMaterialNamesCStr();
		m_FreeSlots.push_back(index);
	}

	void MaterialLibrary::UpdateMaterialNamesCStr()
	{
		m_MaterialNamesCStr.clear();
		if (m_MaterialNamesCStr.size() < m_Materials.size())
			m_MaterialNamesCStr.resize(m_Materials.size());
		for (auto& [key, index] : m_MaterialNameToIndexMap)
		{
			m_MaterialNamesCStr[index] = key.key.c_str();
		}
	}

	const Material* MaterialLibrary::GetMaterial(const MaterialLibraryKey& materialLibraryKey)
	{

		auto it = m_MaterialNameToIndexMap.find(materialLibraryKey);
		if (it != m_MaterialNameToIndexMap.end())
			return &m_Materials[it->second];
		else
			return nullptr;
	}

	const MaterialHandle* MaterialLibrary::GetMaterialHandle(const MaterialLibraryKey& materialLibraryKey)
	{
		auto it = m_MaterialNameToIndexMap.find(materialLibraryKey);
		if (it != m_MaterialNameToIndexMap.end())
			return &m_MaterialHandles[it->second];
		else
			return nullptr;
	}

	void MaterialLibrary::UpdateMaterialName(const MaterialLibraryKey& oldKey, const MaterialLibraryKey& newKey)
	{
		if (oldKey.key == "Default")
			return;

		auto it1 = m_MaterialNameToIndexMap.find(oldKey);
		if (it1 == m_MaterialNameToIndexMap.end())
			return;

		uint32_t index = it1->second;
		m_MaterialNameToIndexMap.erase(it1);

		auto it2 = m_MaterialIndexToNameMap.find(index);
		if (it2 != m_MaterialIndexToNameMap.end())
		{
			m_MaterialIndexToNameMap.erase(it2);
		}

		m_MaterialNameToIndexMap[newKey] = index;
		m_MaterialIndexToNameMap[index] = newKey;

		UpdateMaterialNamesCStr();
	}

	MaterialLibraryKey* MaterialLibrary::GetKey(uint32_t index)
	{
		auto it = m_MaterialIndexToNameMap.find(index);
		if (it != m_MaterialIndexToNameMap.end())
			return &it->second;
		else
			return nullptr;
	}

	uint32_t MaterialLibrary::GetIndex(const MaterialLibraryKey& materialLibraryKey)
	{
		auto it = m_MaterialNameToIndexMap.find(materialLibraryKey);
		if (it != m_MaterialNameToIndexMap.end())
			return it->second;
		else
			return 0;

	}

	void MaterialLibrary::Copy(const MaterialLibrary& materialLibrary)
	{
		m_Materials              = materialLibrary.m_Materials;
		m_MaterialHandles        = materialLibrary.m_MaterialHandles;
		m_MaterialNameToIndexMap = materialLibrary.m_MaterialNameToIndexMap;
		m_MaterialIndexToNameMap = materialLibrary.m_MaterialIndexToNameMap;
		UpdateMaterialNamesCStr();
		m_FreeSlots              = materialLibrary.m_FreeSlots;
	}


	void MaterialLibrary::UpdateMaterial(const MaterialLibraryKey& materialLibraryKey, MaterialType type, TextureLibraryKey& textureLibraryKey)
	{
		if (materialLibraryKey.key == "Default")
			return;

		uint32_t index = GetIndex(materialLibraryKey);

		Material* materialPtr = &m_Materials[index];
		MaterialHandle* materialHandlePtr = &m_MaterialHandles[index];

		std::array<TextureLibraryKey*, s_MaterialTypeCount> materialTemp = {
			&materialPtr->diffuse,
			&materialPtr->specular,
			&materialPtr->normal,
			&materialPtr->parallax,
			&materialPtr->roughness,
			&materialPtr->ao,
			&materialPtr->emission
		};
		std::array<uint64_t*, s_MaterialTypeCount> materialHandleTemp = {
			&materialHandlePtr->diffuse,
			&materialHandlePtr->specular,
			&materialHandlePtr->normal,
			&materialHandlePtr->parallax,
			&materialHandlePtr->roughness,
			&materialHandlePtr->ao,
			&materialHandlePtr->emission
		};

		if ((uint8_t)type < s_MaterialTypeCount)
		{
			*materialHandleTemp[(uint8_t)type] = Texture::GetTextureLibrary()->Get(textureLibraryKey.key, textureLibraryKey.keyHash, textureLibraryKey.flip, textureLibraryKey.internalFormat)->GetHandle();
			*materialTemp[(uint8_t)type] = textureLibraryKey;
		}
	}

	void MaterialLibrary::UpdateMaterial(const MaterialLibraryKey& materialLibraryKey, int lightingMode)
	{
		if (materialLibraryKey.key == "Default")
			return;

		uint32_t index = GetIndex(materialLibraryKey);

		Material* materialPtr = &m_Materials[index];

		materialPtr->lightingMode = lightingMode;
	}

	void MaterialLibrary::UpdateMaterial(const MaterialLibraryKey& materialLibraryKey, Material material)
	{
		if (!IsMaterialValid(material) || materialLibraryKey.key == "Default")
			return;

		uint32_t index = GetIndex(materialLibraryKey);

		Material* materialPtr = &m_Materials[index];
		*materialPtr = material;
		MaterialHandle* materialHandlePtr = &m_MaterialHandles[index];
		*materialHandlePtr = MaterialToMaterialHandle(*materialPtr);

	}

	void MaterialLibrary::UpdateMaterial(uint32_t index, MaterialType type, TextureLibraryKey& textureLibraryKey)
	{
		auto* materialLibrarykey = GetKey(index);
		if (materialLibrarykey != nullptr)
		{
			UpdateMaterial(*materialLibrarykey, type, textureLibraryKey);
		}
	}

	void MaterialLibrary::UpdateMaterial(uint32_t index, int lightingMode)
	{
		auto* materialLibrarykey = GetKey(index);
		if (materialLibrarykey != nullptr)
		{
			UpdateMaterial(*materialLibrarykey, lightingMode);
		}
	}

	void MaterialLibrary::UpdateMaterial(uint32_t index, Material material)
	{
		auto* materialLibrarykey = GetKey(index);
		if (materialLibrarykey != nullptr)
		{
			UpdateMaterial(*materialLibrarykey, material);
		}
	}

	MaterialHandle MaterialLibrary::MaterialToMaterialHandle(Material& material)
	{
		MaterialHandle materialHandle;
		
		materialHandle.diffuse   =  Texture::GetTextureLibrary()->Get(material.diffuse.key, material.diffuse.keyHash, material.diffuse.flip)->GetHandle();
		materialHandle.specular  = 	Texture::GetTextureLibrary()->Get(material.specular.key, material.specular.keyHash, material.specular.flip)->GetHandle();
		materialHandle.normal    =  Texture::GetTextureLibrary()->Get(material.normal.key, material.normal.keyHash, material.normal.flip)->GetHandle();
		materialHandle.parallax  = 	Texture::GetTextureLibrary()->Get(material.parallax.key, material.parallax.keyHash, material.parallax.flip)->GetHandle();
		materialHandle.roughness =	Texture::GetTextureLibrary()->Get(material.roughness.key, material.roughness.keyHash, material.roughness.flip)->GetHandle();
		materialHandle.ao        =	Texture::GetTextureLibrary()->Get(material.ao.key, material.ao.keyHash, material.ao.flip)->GetHandle();
		materialHandle.emission  = 	Texture::GetTextureLibrary()->Get(material.emission.key, material.emission.keyHash, material.emission.flip)->GetHandle();
		
		return materialHandle;
	}

	bool MaterialLibrary::IsMaterialValid(const Material& material)
	{
		if (material.diffuse.key.empty() ||
			material.specular.key.empty() ||
			material.normal.key.empty() ||
			material.parallax.key.empty() ||
			material.roughness.key.empty() ||
			material.ao.key.empty() ||
			material.emission.key.empty()
			)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

}