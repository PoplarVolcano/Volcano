#pragma once

#include "Volcano/Core/Core.h"

namespace Volcano
{
	class VOL_API AppPath
	{
	public:
		static AppPath& GetInstance();
		std::filesystem::path GetExePath() { return m_ExePath; }
		void SetExePath(std::filesystem::path exeAbsolutePath) { m_ExePath = exeAbsolutePath; }
	private:
		std::filesystem::path m_ExePath;
	};
}