#pragma once

namespace Volcano {

	class Input {
	public:
		static bool IsClicked(int code);
		static bool IsKeyPressed(int keycode);
		static bool IsMouseButtonPressed(int mouseButton);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

		static void UpdateClickMap();
		static void Click(int code);
	private:
		static std::unordered_set<int> m_ClickedMap;
		static std::unordered_set<int> m_ClickedMapBuffer;
	};
}