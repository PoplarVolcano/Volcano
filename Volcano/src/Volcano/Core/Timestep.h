#pragma once

namespace Volcano {

	class Timestep
	{
	public:
		Timestep() {}
		Timestep(float time);

		float GetSeconds() const { return m_Time; }
		float GetMilliseconds() const { return m_Time * 1000.0f; }

		//ǿ��ת��Ϊfloatʱ����m_Time
		operator float() const { return m_Time; }
	private:
		float m_Time = 0.0f;
	};
}