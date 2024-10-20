#include "volpch.h"
#include "Project.h"

#include "ProjectSerializer.h"

namespace Volcano {

	void CreateProjectPath(std::filesystem::path projectPath)
	{
		if (!std::filesystem::exists(projectPath.parent_path()))
			CreateProjectPath(projectPath.parent_path());
		if (!std::filesystem::exists(projectPath))
		    std::filesystem::create_directory(projectPath);

	}
	Ref<Project> Project::New(std::filesystem::path newProjectPath, const std::string name)
	{
		Ref<Project> project = CreateRef<Project>();
		ProjectConfig projectConfig;
		projectConfig.Name = name;
		projectConfig.StartScene;
		projectConfig.AssetDirectory = "Assets";
		projectConfig.ScriptModulePath;
		project->SetConfig(projectConfig);
		project->m_ProjectDirectory = newProjectPath;
		CreateProjectPath(newProjectPath);
		CreateProjectPath(newProjectPath / projectConfig.AssetDirectory.string());
		ProjectSerializer serializer(project);
		std::filesystem::path projectFilePath = newProjectPath / (name + ".hproj");
		serializer.Serialize(projectFilePath);
		
		return project;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();

		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			project->m_ProjectDirectory = path.parent_path();
			s_ActiveProject = project;
			return s_ActiveProject;
		}

		return nullptr;
	}

	bool Project::SaveActive(Scene& scene)
	{
		ProjectSerializer serializer(s_ActiveProject);
		return serializer.Serialize(s_ActiveProject->GetProjectDirectory().append(s_ActiveProject->GetConfig().Name + ".hproj"));
	}

}