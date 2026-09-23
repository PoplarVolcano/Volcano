#include "volpch.h"
#include "SceneSerializer.h"

#include "Volcano/Scene/Entity.h"
#include "Volcano/Utils/FileUtils.h"
#include "Volcano/Core/AppPath.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Volcano {

	namespace Utils
	{
		void SerializeEntity(YAML::Emitter& out, Entity& entity);
		Ref<Entity> DeserializeEntity(YAML::Node& entity, Scene* scene, Ref<Entity> entityParent = nullptr);
	}


	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Scene" << YAML::Value << m_Scene->GetName();
			out << YAML::Key << "HDREnabled" << YAML::Value << m_Scene->GetHDREnabled();
			out << YAML::Key << "Exposure" << YAML::Value << m_Scene->GetExposure();
			out << YAML::Key << "BloomEnabled" << YAML::Value << m_Scene->GetBloomEnabled();

			out << YAML::Key << "MaterialLibrary" << YAML::Value;
			out << YAML::BeginSeq; // MaterialLibrary
			{
				auto& materialLibraryNameMap = m_Scene->GetMaterialLibrary()->GetMaterialNameMap();

				for (auto& [key, index] : materialLibraryNameMap)
				{
					out << YAML::BeginMap;//key
					{
						auto* material = m_Scene->GetMaterialLibrary()->GetMaterial(key);
						out << YAML::Key << "key"           << YAML::Value << key.key;
						out << YAML::Key << "index"         << YAML::Value << index;
						out << YAML::Key << "diffuseKey"    << YAML::Value << material->diffuse.key;
						out << YAML::Key << "diffuseFlip"   << YAML::Value << material->diffuse.flip;
						out << YAML::Key << "specularKey"   << YAML::Value << material->specular.key;
						out << YAML::Key << "specularFlip"  << YAML::Value << material->specular.flip;
						out << YAML::Key << "normalKey"     << YAML::Value << material->normal.key;
						out << YAML::Key << "normalFlip"    << YAML::Value << material->normal.flip;
						out << YAML::Key << "parallaxKey"   << YAML::Value << material->parallax.key;
						out << YAML::Key << "parallaxFlip"  << YAML::Value << material->parallax.flip;
						out << YAML::Key << "roughnessKey"  << YAML::Value << material->roughness.key;
						out << YAML::Key << "roughnessFlip" << YAML::Value << material->roughness.flip;
						out << YAML::Key << "aoKey"         << YAML::Value << material->ao.key;
						out << YAML::Key << "aoFlip"        << YAML::Value << material->ao.flip;
						out << YAML::Key << "emissionKey"   << YAML::Value << material->emission.key;
						out << YAML::Key << "emissionFlip"  << YAML::Value << material->emission.flip;
						out << YAML::Key << "lightingMode"  << YAML::Value << material->lightingMode;
						out << YAML::EndMap;//key
					}
				}
				out << YAML::EndSeq; // MaterialLibrary
			}

			if (!m_Scene->GetEntityList().empty())
			{
				out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;// 开始序列化
				{
					for (auto& entity : m_Scene->GetEntityList())
						Utils::SerializeEntity(out, *entity.get());
					out << YAML::EndSeq; // 结束序列化
				}
			}
		}
		out << YAML::EndMap;



		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			VOL_CORE_ERROR("Failed to load .volcano file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;
		std::string sceneName = data["Scene"].as<std::string>();
		VOL_CORE_TRACE("Deserializing scene '{0}'", sceneName);
		m_Scene->SetName(sceneName);
		auto sceneFileRelativePath = std::filesystem::relative(filepath, AppPath::GetInstance().GetExePath());
		m_Scene->SetFileRelativePath(sceneFileRelativePath.string());

		m_Scene->SetHDREnabled(data["HDREnabled"].as<bool>());
		m_Scene->SetExposure(data["Exposure"].as<float>());
		m_Scene->SetBloomEnabled(data["BloomEnabled"].as<bool>());

		auto materialLibraryNode = data["MaterialLibrary"];
		if (materialLibraryNode && materialLibraryNode.IsSequence())
		{
			auto* materialLibrary = m_Scene->GetMaterialLibrary();
			for (const auto& materialNode : materialLibraryNode)
			{
				std::string keyStr = materialNode["key"].as<std::string>();
				MaterialLibraryKey key = { keyStr, std::hash<std::string>{}(keyStr) };
				if (key == *materialLibrary->GetKey(0))
					continue;
				uint32_t index = materialNode["index"].as<uint32_t>();
				Material material = {
					{ materialNode["diffuseKey"].as<std::string>(),   materialNode["diffuseFlip"].as<bool>()   },
				    { materialNode["specularKey"].as<std::string>(),  materialNode["specularFlip"].as<bool>()  },
				    { materialNode["normalKey"].as<std::string>(),    materialNode["normalFlip"].as<bool>()    },
				    { materialNode["parallaxKey"].as<std::string>(),  materialNode["parallaxFlip"].as<bool>()  },
				    { materialNode["roughnessKey"].as<std::string>(), materialNode["roughnessFlip"].as<bool>() },
				    { materialNode["aoKey"].as<std::string>(),        materialNode["aoFlip"].as<bool>()        },
				    { materialNode["emissionKey"].as<std::string>(),  materialNode["emissionFlip"].as<bool>()  },
					materialNode["lightingMode"].as<int>()
				};
				materialLibrary->AddMaterial(key, index, material);

			}
		}

		auto entities = data["Entities"];
		if (entities)
			for (auto entity : entities)
				Utils::DeserializeEntity(entity, m_Scene.get());
		return true;
	}

}