#pragma once

namespace Volcano{

	class Timestep
	{
	public:
		Timestep(float time = 0.0f)
			:m_Time(time)
		{
		}

		//ǿ��ת��Ϊfloatʱ����m_Time
		operator float() const { return m_Time; }

		float GetSceonds() const { return m_Time; }
		float GetMilliseconds() const { return m_Time * 1000.0f; }
	private:
		float m_Time;
	};
}