#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

namespace Volcano {

	class ParticleSystem;

	struct Particle
	{
		glm::vec3 position;          // 粒子的世界坐标
		glm::vec3 velocity;          // 粒子的当前瞬时速度（向量），决定当前帧的运动方向和速度
		glm::vec3 animatedVelocity;  // 外部驱动力或动画速度，通常是外部系统（如粒子系统脚本）强行注入的偏移量
		glm::vec3 initialVelocity;   // 粒子诞生瞬间的初速度
									 
		glm::quat rotation;          // 粒子的欧拉角（弧度制）
		glm::vec3 axisOfRotation;    // 旋转轴
		float     angularVelocity;   // 角速度（弧度/秒）
		glm::vec3 angularVelocity3D; // 角速度（弧度/秒）
								     
		glm::vec3 startSize;         // 粒子的缩放尺寸
		glm::vec4 startColor;        // 粒子的RGBA颜色
								     
		uint32_t  randomSeed;        // 该粒子独有的随机种子
		uint32_t  parentRandomSeed;  // 父级粒子的随机种子
								     
		float     lifetime;          // 当前剩余存活时间（秒）
		float     startLifetime;     // 粒子诞生时的总寿命长度
									 
		float     emitAccumulator0;  // “速率时间累计器”（累积时间达到发射间隔则喷出一个粒子）
		float     emitAccumulator1;  // “突发(Burst)计数器” 或 “距离累计器”（例如每移动1米发射一个粒子）
									 
		uint8_t   meshType;			 
		uint32_t  flags;             // 位掩码（Bitmask），用于存储该粒子的布尔状态，例如：是否激活、是否被冻结、是否受光照影响、是否在视锥裁剪中。

		ParticleSystem* particleSystem;

		Particle(ParticleSystem* ps)
		{
			position          = { 0.0f, 0.0f,  0.0f };
			velocity          = { 0.0f, 0.0f, -1.0f };
			animatedVelocity  = { 0.0f, 0.0f,  0.0f };
			initialVelocity   = { 0.0f, 0.0f,  0.0f };
			rotation          = glm::quat(glm::vec3(0.0f));
			axisOfRotation    = { 0.0f, 0.0f, -1.0f };
			angularVelocity   = 0.0f;
			angularVelocity3D = { 0.0f, 0.0f,  0.0f };
			startSize         = { 0.0f, 0.0f,  0.0f };
			startColor        = { 0.0f, 0.0f,  0.0f, 0.0f };
			randomSeed        = 0;
			parentRandomSeed  = 0;
			lifetime          = 0.0f;
			startLifetime     = 0.0f;
			emitAccumulator0  = 0.0f;
			emitAccumulator1  = 0.0f;
			meshType          = 0;
			flags             = 0;

			particleSystem = ps;
		}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(rotation));

			return glm::translate(glm::mat4(1.0f), position)
				* rotation
				* glm::scale(glm::mat4(1.0f), startSize);
		}

		glm::mat3 GetNormalTransform() const
		{
			return  glm::mat3(transpose(inverse(GetTransform())));
		}

		void Update();
		void Render();
	};

}