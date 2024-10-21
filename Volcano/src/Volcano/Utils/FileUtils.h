#pragma once

namespace Volcano {

	class FileUtils
	{
	public:
		static void CreatePath(std::filesystem::path path);
		static std::string GetFileNameFromPath(const std::string path);
	};

}