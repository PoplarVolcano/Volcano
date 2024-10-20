#pragma once
#include "Volcano/Core/Base.h"
#include "Volcano/Scene/Scene.h"

namespace Volcano {

	// 项目配置
	struct ProjectConfig
	{
		ProjectConfig() = default;
		std::string Name = "Untitled";

		// 项目开启场景路径
		std::filesystem::path StartScene;
		// 项目资源路径
		std::filesystem::path AssetDirectory;
		// 项目脚本dll路径
		std::filesystem::path ScriptModulePath;
	};

	
	class Project
	{
	public:
		static std::filesystem::path GetProjectDirectory()
		{
			if(s_ActiveProject)
				return s_ActiveProject->m_ProjectDirectory;
			VOL_CORE_TRACE("GetProjectDirectory::bug");
		}

		static std::filesystem::path GetAssetDirectory()
		{
			if (s_ActiveProject)
				return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
			VOL_CORE_TRACE("GetAssetDirectory::bug");
		}

		// TODO(Yan): move to asset manager when we have one
		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			if (s_ActiveProject)
				return GetAssetDirectory() / path;
			VOL_CORE_TRACE("GetAssetFileSystemPath::bug");
		}

		ProjectConfig& GetConfig() { return m_Config; }
		void SetConfig(ProjectConfig config) { m_Config = config; }

		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<Project> New(std::filesystem::path newProjectPath, const std::string name);
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(Scene& scene);
	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory; // project文件所在目录

		inline static Ref<Project> s_ActiveProject;
	};
	
}