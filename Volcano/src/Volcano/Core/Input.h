#pragma once

namespace Volcano {

	class Input {
	public:
		static bool IsKeyPressed(int keycode);
		static bool IsMouseButtonPressed(int mouseButton);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}