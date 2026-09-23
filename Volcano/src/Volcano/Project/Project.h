#pragma once
#include "Volcano/Core/Core.h"
#include "Volcano/Scene/Scene.h"

namespace Volcano {

	// 项目配置
	struct VOL_API ProjectConfig
	{
		ProjectConfig() = default;
		std::string name = "Untitled";
		// 项目启动时场景相对路径，相对项目目录
		std::filesystem::path startScene;
		// 项目资源相对路径，相对项目目录
		std::filesystem::path assetRelativePath;
		// 项目脚本dll路径
		std::filesystem::path scriptModulePath;
	};


	class VOL_API Project
	{
	public:
		static std::filesystem::path GetProjectAbsolutePath();
		static std::filesystem::path GetProjectRelativePath();
		static std::filesystem::path GetAssetsAbsolutePath();
		static std::filesystem::path GetAssetsRelativePath();
		static std::filesystem::path GetRelativeAssetDirectory(std::filesystem::path path);
		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path);

		ProjectConfig& GetConfig() { return m_Config; }
		void SetConfig(ProjectConfig config) { m_Config = config; }

		// 获取项目，如果没有设置项目则返回空指针
		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<Project> New(std::filesystem::path newProjectAbsolutePath, const std::string name);
		static Ref<Project> Load(const std::filesystem::path& projectFileAbsolutePath);
		static bool SaveActive();

		// 是否启动游戏
		bool GetPlayGame() { return m_PlayGame; }
		void SetPlayGame(bool playGame) { m_PlayGame = playGame; }
	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectRelativePath; // project文件所在相对目录，相对exe
		std::filesystem::path m_ProjectFileRelativePath;

		inline static Ref<Project> s_ActiveProject;

		bool m_PlayGame = false;
	};

}