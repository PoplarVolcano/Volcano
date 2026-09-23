#pragma once

#include "Volcano/Core/Core.h"

namespace Volcano {

	class VOL_API FileUtils
	{
	public:
		static void CreatePath(std::filesystem::path path);
		static std::string GetFileNameFromPath(const std::string path);
	};

}