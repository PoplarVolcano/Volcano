#include "volpch.h"
#include "Volcano/Core/Window.h"

#ifdef VOL_PLATFORM_WINDOWS
#include "Volcano/Platform/Windows/WindowsWindow.h"
#endif

namespace Volcano
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef VOL_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
#else
		VOL_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}

}