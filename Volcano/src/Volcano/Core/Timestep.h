#pragma once

namespace Volcano {

	class Timestep
	{
	public:
		Timestep() {}
		Timestep(float time);

		float GetSeconds() const { return m_Time; }
		float GetMilliseconds() const { return m_Time * 1000.0f; }

		//强制转换为float时返回m_Time
		operator float() const { return m_Time; }
	private:
		float m_Time = 0.0f;
	};
}