#include "volpch.h"
#include "Project.h"
#include "Volcano/Utils/FileUtils.h"
#include "ProjectSerializer.h"
#include "Volcano/Core/AppPath.h"

namespace Volcano {

	std::filesystem::path Project::GetProjectAbsolutePath()
	{
		if (s_ActiveProject)
			return AppPath::GetInstance().GetExePath() / s_ActiveProject->m_ProjectRelativePath;
		VOL_CORE_TRACE("GetProjectAbsolutePath::bug");
		return {};
	}

	std::filesystem::path Project::GetProjectRelativePath()
	{
		if (s_ActiveProject)
			return s_ActiveProject->m_ProjectRelativePath;
		VOL_CORE_TRACE("GetProjectAbsolutePath::bug");
		return {};
	}

	std::filesystem::path Project::GetAssetsAbsolutePath()
	{
		if (s_ActiveProject)
			return GetProjectAbsolutePath() / s_ActiveProject->m_Config.assetRelativePath;
		VOL_CORE_TRACE("GetAssetsAbsolutePath::bug");
		return {};
	}

	std::filesystem::path Project::GetAssetsRelativePath()
	{
		if (s_ActiveProject)
			return s_ActiveProject->m_Config.assetRelativePath;
		VOL_CORE_TRACE("GetAssetsRelativePath::bug");
		return {};
	}

	std::filesystem::path Project::GetRelativeAssetDirectory(std::filesystem::path path)
	{
		if (s_ActiveProject)
			return std::filesystem::relative(path, GetAssetsAbsolutePath());
		VOL_CORE_TRACE("GetRelativeAssetDirectory::bug");
		return {};
	}

	std::filesystem::path Project::GetAssetFileSystemPath(const std::filesystem::path& path)
	{
		if (s_ActiveProject)
			return GetAssetsAbsolutePath() / path;
		VOL_CORE_TRACE("GetAssetFileSystemPath::bug");
		return {};
	}

	Ref<Project> Project::New(std::filesystem::path newProjectAbsolutePath, const std::string projectName)
	{
		Ref<Project> project = CreateRef<Project>();
		ProjectConfig projectConfig;
		projectConfig.name = projectName;
		projectConfig.startScene;
		projectConfig.assetRelativePath = "Assets";
		projectConfig.scriptModulePath;
		project->SetConfig(projectConfig);
		project->m_ProjectRelativePath = std::filesystem::relative(newProjectAbsolutePath, AppPath::GetInstance().GetExePath());
		std::filesystem::path projectAbsolutePath = AppPath::GetInstance().GetExePath() / newProjectAbsolutePath;
		FileUtils::CreatePath(projectAbsolutePath);
		FileUtils::CreatePath(projectAbsolutePath / projectConfig.assetRelativePath.string());
		FileUtils::CreatePath(projectAbsolutePath / projectConfig.assetRelativePath.string() / "Scenes");
		ProjectSerializer serializer(project);
		project->m_ProjectFileRelativePath = project->m_ProjectRelativePath / (projectName + ".proj");
		auto projectFileAbsolutePath = AppPath::GetInstance().GetExePath() / project->m_ProjectFileRelativePath;
		serializer.Serialize(projectFileAbsolutePath);

		return project;
	}

	Ref<Project> Project::Load(const std::filesystem::path& projectFileAbsolutePath)
	{
		Ref<Project> project = CreateRef<Project>();

		ProjectSerializer serializer(project);
		if (serializer.Deserialize(projectFileAbsolutePath))
		{
			project->m_ProjectRelativePath = std::filesystem::relative(projectFileAbsolutePath.parent_path(), AppPath::GetInstance().GetExePath());
			project->m_ProjectFileRelativePath = std::filesystem::relative(projectFileAbsolutePath, AppPath::GetInstance().GetExePath());
			s_ActiveProject = project;
			return s_ActiveProject;
		}

		return nullptr;
	}

	bool Project::SaveActive()
	{
		ProjectSerializer serializer(s_ActiveProject);
		return serializer.Serialize(AppPath::GetInstance().GetExePath() / s_ActiveProject->m_ProjectFileRelativePath);
	}

}