#pragma once

namespace Volcano {

	class MouseBuffer 
	{
	private:
		MouseBuffer() : m_OnActive(true) {};
		~MouseBuffer() {};
		bool m_OnActive;
		
	public:
		static MouseBuffer& instance() {
			static MouseBuffer* instance = new MouseBuffer();
			return *instance;
		}

		inline bool GetOnActive() { return m_OnActive; }
		inline void SetOnActive(bool onActive) { m_OnActive = onActive; }
	};
}