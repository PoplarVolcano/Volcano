#include "volpch.h"
#include "ProjectSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Volcano {

	ProjectSerializer::ProjectSerializer(Ref<Project> project)
		: m_Project(project)
	{
	}

	bool ProjectSerializer::Serialize(const std::filesystem::path& projectFileAbsolutePath)
	{
		const auto& config = m_Project->GetConfig();

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap;// Project
				out << YAML::Key << "name" << YAML::Value << config.name;
				out << YAML::Key << "startScene" << YAML::Value << config.startScene.string();
				out << YAML::Key << "assetRelativePath" << YAML::Value << config.assetRelativePath.string();
				out << YAML::Key << "scriptModulePath" << YAML::Value << config.scriptModulePath.string();
				out << YAML::EndMap; // Project
			}
			out << YAML::EndMap; // Root
		}

		std::ofstream fout(projectFileAbsolutePath);
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& projectFileAbsolutePath)
	{
		auto& config = m_Project->GetConfig();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(projectFileAbsolutePath.string());
		}
		catch (YAML::ParserException e)
		{
			//VOL_CORE_ERROR("Failed to load project file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		auto projectNode = data["Project"];
		if (!projectNode)
			return false;

		config.name = projectNode["name"].as<std::string>();
		config.startScene = projectNode["startScene"].as<std::string>();
		config.assetRelativePath = projectNode["assetRelativePath"].as<std::string>();
		config.scriptModulePath = projectNode["scriptModulePath"].as<std::string>();
		return true;
	}

}