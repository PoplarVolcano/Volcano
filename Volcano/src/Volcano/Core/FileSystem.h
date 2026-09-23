#pragma once

#include "Volcano/Core/Buffer.h"

namespace Volcano {

	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
	};

}