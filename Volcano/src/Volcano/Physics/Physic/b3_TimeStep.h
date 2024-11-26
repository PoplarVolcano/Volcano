#pragma once

#include "b3_Math.h"

namespace Volcano {

	// �������ݡ�ʱ�䵥λΪ���롣 Profiling data. Times are in milliseconds.
	struct b3_Profile
	{
		float step;          // ʱ�䲽��ʼ��ʱ�䲽��ɵ�deltaTime
		float collide;       // ��ʼ������ײ��������ɵ�deltaTime
		float solve;         // ��ʼsolve��solve��ɵ�deltaTime
		float solveInit;
		float solveVelocity;
		float solvePosition;
		float broadphase;
		float solveTOI;      // ��ʼsolveTOI��solveTOI��ɵ�deltaTime
	};

	struct b3_TimeStep
	{
		float deltaTime;	    // time step
		float inv_deltaTime;    // inverse time step (0 if deltaTime == 0).
		float deltaTimeRatio;   // deltaTime * inv_deltaTime0
		int velocityIterations; // ���ʵ�����
		int positionIterations; // λ�õ�����
		bool warmStarting;      // ������
	};

	struct b3_Position
	{
		glm::vec3 position;
		glm::vec3 rotation;
	};
	struct b3_Velocity
	{
		glm::vec3 linearVelocity;
		glm::vec3 angularVelocity;
	};

	/// ��������� Solver Data
	struct b3_SolverData
	{
		b3_TimeStep step;
		b3_Position* positions;  // λ�����飬b3_Island::Solve�б���m_bodies����sweep.center��sweep.rotationע��positions
		b3_Velocity* velocities; // �ٶ����飬b3_Island::Solve�б���m_bodies�������������ٶȽ��ٶ�ע��velocities
	};

}