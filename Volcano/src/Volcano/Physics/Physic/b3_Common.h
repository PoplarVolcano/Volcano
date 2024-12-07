#pragma once

#include "b3_Setting.h"

#define b3_PI			3.14159265359f
#define	b3_Epsilon		FLT_EPSILON  // 表示 1.0 与比 1.0 大的最小浮点数之间的差。这个值通常用于浮点数的比较，因为直接比较浮点数可能会因为精度问题而导致错误的结果。

// Collision

// 两个面之间的最大接触点数量。不要更改此值。The maximum number of contact points between two convex shapes. Do not change this value.
#define b3_MaxManifoldPoints	12

// 这用于在动态树中扩增AABB。这允许代理少量移动，而不会触发树调整。 这是以米为单位的。
#define b3_AABBExtension		(0.1f * b3_LengthUnitsPerMeter)

// 这用于在动态树中扩增AABB。这用于根据当前位移(displacement)预测未来位置。这是一个无量纲的乘数(dimensionless multiplier)。
#define b3_AABBMultiplier		4.0f

// 用作碰撞和约束公差(constraint tolerance)的小长度。通常，它被选择为具有数字(numerically)意义，但在视觉上无关紧要。以米为单位。
#define b3_LinearSlop			(0.005f * b3_LengthUnitsPerMeter)

// 用作碰撞和约束公差的小角度。通常，它被选择为具有数字意义，但在视觉上无关紧要。
#define b3_AngularSlop			(2.0f / 180.0f * b3_PI)

// 多边形/边缘形状蒙皮(polygon/edge shape skin)的半径。这不应该被修改。
// 减小此值意味着多边形将没有足够(insufficient)的缓冲区来进行连续碰撞。将其放大可能会产生顶点碰撞的伪影(artifacts for vertex collision)。
#define b3_BoxRadius		    (2.0f * b3_LinearSlop)

// 连续物理模拟中每个contact的最大子步骤数
#define b3_MaxSubSteps			8


// Dynamics

// 解决TOI影响所需处理的最大contacts数量。
#define b3_MaxTOIContacts			32

// 求解约束时使用的最大线性位置校正(correction)。这有助于防止超调(overshoot)。米。
#define b3_MaxLinearCorrection		(0.2f * b3_LengthUnitsPerMeter)

// 求解约束时使用的最大角度位置校正(correction)。这有助于防止超调(overshoot)。弧度。
#define b3_MaxAngularCorrection		(8.0f / 180.0f * b3_PI)

// body每一步的最大线性平移。这个限制非常大，用于防止数值(numerical)问题。你不需要调整这个。米。
#define b3_MaxTranslation			(2.0f * b3_LengthUnitsPerMeter)
#define b3_MaxTranslationSquared	(b3_MaxTranslation * b3_MaxTranslation)

// body的最大角速度。这个限制非常大，用于防止数值问题。你不需要调整这个。
#define b3_MaxRotation				(0.5f * b3_PI)
#define b3_MaxRotationSquared		(b3_MaxRotation * b3_MaxRotation)

// 此比例(scale)因子控制结算重叠的速度。理想情况下，这将是1，以便在一个时间步长内消除重叠。然而，使用接近1的值通常会导致超调(overshoot)。
#define b3_Baumgarte				0.2f
#define b3_TOIBaumgarte				0.75f


// Sleep

// body睡眠前必须静止的时间。
#define b3_TimeToSleep				0.5f

// 如果body的线速度超过这个容差(tolerance)，它就不能睡眠。
#define b3_LinearSleepTolerance		(0.01f * b3_LengthUnitsPerMeter)

// 如果body的角速度超过这个容差(tolerance)，它就不能睡眠。
#define b3_AngularSleepTolerance	(2.0f / 180.0f * b3_PI)
