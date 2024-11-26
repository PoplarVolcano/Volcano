#pragma once

#include "b3_Distance.h"
#include "b3_Math.h"

namespace Volcano {

	// b3_TimeOfImpact函数用于确定两个形状运动时碰撞的时间。这称为撞击时间(time of impact, TOI)。
	// b3_TimeOfImpact的主要目地是防止隧穿效应。特别是，它设计来防止运动的物体隧穿过静态的几何形状而出到外面。

	// b3_TimeOfImpact的输入参数 
	struct b3_TOIInput
	{
		b3_DistanceProxy proxyA;   // 距离代理
		b3_DistanceProxy proxyB;
		b3_Sweep sweepA;
		b3_Sweep sweepB;
		float tMax;		// 定义扫描间隔(sweep interval)[0，tMax]
	};

	// b3_TimeOfImpact的输出参数。
	struct b3_TOIOutput
	{
		enum State
		{
			e_unknown,
			e_failed,
			e_overlapped, // AABB重叠
			e_touching,   // 未相交
			e_separated   // 分开
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