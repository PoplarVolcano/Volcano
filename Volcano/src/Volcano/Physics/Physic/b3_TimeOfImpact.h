#pragma once

#include "b3_Distance.h"
#include "b3_Math.h"

namespace Volcano {

	// b3_TimeOfImpact��������ȷ��������״�˶�ʱ��ײ��ʱ�䡣���Ϊײ��ʱ��(time of impact, TOI)��
	// b3_TimeOfImpact����ҪĿ���Ƿ�ֹ��ЧӦ���ر��ǣ����������ֹ�˶�������������̬�ļ�����״���������档

	// b3_TimeOfImpact��������� 
	struct b3_TOIInput
	{
		b3_DistanceProxy proxyA;   // �������
		b3_DistanceProxy proxyB;
		b3_Sweep sweepA;
		b3_Sweep sweepB;
		float tMax;		// ����ɨ����(sweep interval)[0��tMax]
	};

	// b3_TimeOfImpact�����������
	struct b3_TOIOutput
	{
		enum State
		{
			e_unknown,
			e_failed,
			e_overlapped, // AABB�ص�
			e_touching,   // δ�ཻ
			e_separated   // �ֿ�
		};

		State state;
		float t;
	};

	// Compute the upper bound on time before two shapes penetrate. Time is represented as
	// a fraction between [0,tMax]. This uses a swept separating axis and may miss some intermediate,
	// non-tunneling collisions. If you change the time interval, you should call this function
	// again.
	// Note: use b3_Distance to compute the contact point and normal at the time of impact.
	void b3_TimeOfImpact(b3_TOIOutput* output, const b3_TOIInput* input);

}