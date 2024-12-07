#pragma once

#include "b3_Setting.h"

#define b3_PI			3.14159265359f
#define	b3_Epsilon		FLT_EPSILON  // ��ʾ 1.0 ��� 1.0 �����С������֮��Ĳ���ֵͨ�����ڸ������ıȽϣ���Ϊֱ�ӱȽϸ��������ܻ���Ϊ������������´���Ľ����

// Collision

// ������֮������Ӵ�����������Ҫ���Ĵ�ֵ��The maximum number of contact points between two convex shapes. Do not change this value.
#define b3_MaxManifoldPoints	12

// �������ڶ�̬��������AABB����������������ƶ��������ᴥ���������� ��������Ϊ��λ�ġ�
#define b3_AABBExtension		(0.1f * b3_LengthUnitsPerMeter)

// �������ڶ�̬��������AABB�������ڸ��ݵ�ǰλ��(displacement)Ԥ��δ��λ�á�����һ�������ٵĳ���(dimensionless multiplier)��
#define b3_AABBMultiplier		4.0f

// ������ײ��Լ������(constraint tolerance)��С���ȡ�ͨ��������ѡ��Ϊ��������(numerically)���壬�����Ӿ����޹ؽ�Ҫ������Ϊ��λ��
#define b3_LinearSlop			(0.005f * b3_LengthUnitsPerMeter)

// ������ײ��Լ�������С�Ƕȡ�ͨ��������ѡ��Ϊ�����������壬�����Ӿ����޹ؽ�Ҫ��
#define b3_AngularSlop			(2.0f / 180.0f * b3_PI)

// �����/��Ե��״��Ƥ(polygon/edge shape skin)�İ뾶���ⲻӦ�ñ��޸ġ�
// ��С��ֵ��ζ�Ŷ���ν�û���㹻(insufficient)�Ļ�����������������ײ������Ŵ���ܻ����������ײ��αӰ(artifacts for vertex collision)��
#define b3_BoxRadius		    (2.0f * b3_LinearSlop)

// ��������ģ����ÿ��contact������Ӳ�����
#define b3_MaxSubSteps			8


// Dynamics

// ���TOIӰ�����账������contacts������
#define b3_MaxTOIContacts			32

// ���Լ��ʱʹ�õ��������λ��У��(correction)���������ڷ�ֹ����(overshoot)���ס�
#define b3_MaxLinearCorrection		(0.2f * b3_LengthUnitsPerMeter)

// ���Լ��ʱʹ�õ����Ƕ�λ��У��(correction)���������ڷ�ֹ����(overshoot)�����ȡ�
#define b3_MaxAngularCorrection		(8.0f / 180.0f * b3_PI)

// bodyÿһ�����������ƽ�ơ�������Ʒǳ������ڷ�ֹ��ֵ(numerical)���⡣�㲻��Ҫ����������ס�
#define b3_MaxTranslation			(2.0f * b3_LengthUnitsPerMeter)
#define b3_MaxTranslationSquared	(b3_MaxTranslation * b3_MaxTranslation)

// body�������ٶȡ�������Ʒǳ������ڷ�ֹ��ֵ���⡣�㲻��Ҫ���������
#define b3_MaxRotation				(0.5f * b3_PI)
#define b3_MaxRotationSquared		(b3_MaxRotation * b3_MaxRotation)

// �˱���(scale)���ӿ��ƽ����ص����ٶȡ���������£��⽫��1���Ա���һ��ʱ�䲽���������ص���Ȼ����ʹ�ýӽ�1��ֵͨ���ᵼ�³���(overshoot)��
#define b3_Baumgarte				0.2f
#define b3_TOIBaumgarte				0.75f


// Sleep

// body˯��ǰ���뾲ֹ��ʱ�䡣
#define b3_TimeToSleep				0.5f

// ���body�����ٶȳ�������ݲ�(tolerance)�����Ͳ���˯�ߡ�
#define b3_LinearSleepTolerance		(0.01f * b3_LengthUnitsPerMeter)

// ���body�Ľ��ٶȳ�������ݲ�(tolerance)�����Ͳ���˯�ߡ�
#define b3_AngularSleepTolerance	(2.0f / 180.0f * b3_PI)
