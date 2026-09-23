#include "volpch.h"
#include "FileSystem.h"

namespace Volcano {

	Buffer FileSystem::ReadFileBinary(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			return {};
		}

		// 返回当前位置相对文件开头的字节偏移，初始为文件末尾。
		std::streampos end = stream.tellg();
		// 把读指针移到文件开头
		stream.seekg(0, std::ios::beg);
		uint64_t size = end - stream.tellg();

		if (size == 0)
		{
			// File is empty
			return {};
		}

		Buffer buffer(size);
		stream.read(buffer.As<char>(), size);
		stream.close();
		return buffer;
	}

}