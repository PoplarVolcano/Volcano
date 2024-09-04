#pragma once
#include "Volcano/Core/Base.h"

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
		static const std::filesystem::path& GetProjectDirectory()
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

		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);
	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;

		inline static Ref<Project> s_ActiveProject;
	};
	
}