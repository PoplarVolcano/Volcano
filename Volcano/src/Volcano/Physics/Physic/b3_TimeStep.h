#pragma once

#include "b3_Math.h"

namespace Volcano {

	// 分析数据。时间单位为毫秒。 Profiling data. Times are in milliseconds.
	struct b3_Profile
	{
		float step;          // 时间步开始到时间步完成的deltaTime
		float collide;       // 开始计算碰撞到计算完成的deltaTime
		float solve;         // 开始solve到solve完成的deltaTime
		float solveInit;
		float solveVelocity;
		float solvePosition;
		float broadphase;
		float solveTOI;      // 开始solveTOI到solveTOI完成的deltaTime
	};

	struct b3_TimeStep
	{
		float deltaTime;	    // time step
		float inv_deltaTime;    // inverse time step (0 if deltaTime == 0).
		float deltaTimeRatio;   // deltaTime * inv_deltaTime0
		int velocityIterations; // 速率迭代器
		int positionIterations; // 位置迭代器
		bool warmStarting;      // 热启动
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

	/// 求解器数据 Solver Data
	struct b3_SolverData
	{
		b3_TimeStep step;
		b3_Position* positions;  // 位置数组，b3_Island::Solve中遍历m_bodies，将sweep.center和sweep.rotation注入positions
		b3_Velocity* velocities; // 速度数组，b3_Island::Solve中遍历m_bodies，将计算后的线速度角速度注入velocities
	};

}