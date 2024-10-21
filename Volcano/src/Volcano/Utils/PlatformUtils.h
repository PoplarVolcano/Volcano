#pragma once
#include <string>

namespace Volcano {

	class FileDialogs
	{
	public:
		// these returns empty string if cancelled
		static std::string OpenFile(const char* filter, std::string oldPath = std::string());
		static std::string OpenFolder(std::string oldPath);
		static std::string SaveFile(const char* filter, std::string oldPath = std::string());
		static std::filesystem::path GetProjectPath();
	private:
		static std::filesystem::path s_ProjectPath;
	};

	class Time
	{
	public:
		static float GetTime();
	};
}