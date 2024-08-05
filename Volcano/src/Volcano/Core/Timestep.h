#pragma once

namespace Volcano{

	class Timestep
	{
	public:
		Timestep(float time = 0.0f)
			:m_Time(time)
		{
		}

		//强制转换为float时返回m_Time
		operator float() const { return m_Time; }

		float GetSceonds() const { return m_Time; }
		float GetMilliseconds() const { return m_Time * 1000.0f; }
	private:
		float m_Time;
	};
}