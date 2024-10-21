#include "volpch.h"
#include "FileUtils.h"

namespace Volcano {

	void FileUtils::CreatePath(std::filesystem::path path)
	{
		if (!std::filesystem::exists(path.parent_path()))
			CreatePath(path.parent_path());
		if (!std::filesystem::exists(path))
			std::filesystem::create_directory(path);
	}

	// 获取filepath的文件名
	std::string FileUtils::GetFileNameFromPath(const std::string filepath)
	{
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		return filepath.substr(lastSlash, count);
	}
}