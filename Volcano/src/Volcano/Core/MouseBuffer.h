#pragma once

#include "Volcano/Core/Core.h"

namespace Volcano {

	class VOL_API MouseBuffer
	{
	public:
		static MouseBuffer& instance() {
			static MouseBuffer* instance = new MouseBuffer();
			return *instance;
		}

		inline bool GetOnActive() { return m_OnActive; }
		inline void SetOnActive(bool onActive) { m_OnActive = onActive; }

	private:
		MouseBuffer() : m_OnActive(true) {};
		~MouseBuffer() {};
		bool m_OnActive;

	};
}