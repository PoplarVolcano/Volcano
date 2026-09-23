#pragma once

#include "Volcano/Core/Core.h"

namespace Volcano {

	class VOL_API Input {
	public:
		static bool IsClicked(int code);
		static bool IsKeyPressed(int keycode);
		static bool IsMouseButtonPressed(int mouseButton);
		static std::pair<float, float> GetMousePosition();

		// 相对于指定窗口“客户区”（Client Area/Content Area）左上角的“屏幕坐标”（Screen Coordinates）
		// 坐标原点 (0, 0) 位于窗口客户区的左上角
		static float GetMouseX();
		static float GetMouseY();

		static void UpdateClickMap();
		static void Click(int code);
	private:
		static std::unordered_set<int> m_ClickedMap;
		static std::unordered_set<int> m_ClickedMapBuffer;
	};
}