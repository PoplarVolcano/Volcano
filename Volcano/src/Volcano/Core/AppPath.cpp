#include "volpch.h"
#include "AppPath.h"

namespace Volcano
{
	AppPath& AppPath::GetInstance()
	{
		static AppPath instance; // C++11 保证线程安全初始化
		return instance;
	}
}