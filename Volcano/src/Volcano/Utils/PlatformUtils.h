#pragma once
#include <string>

namespace Volcano {

	class FileDialogs
	{
	public:
		// these returns empty string if cancelled
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};

	class Time
	{
	public:
		static float GetTime();
	};
}