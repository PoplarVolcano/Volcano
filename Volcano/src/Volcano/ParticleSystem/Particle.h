#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

namespace Volcano {

	class ParticleSystem;

	struct Particle
	{

		glm::vec3 position;
		glm::vec3 velocity;       // 粒子的运动方向和速度
		glm::vec3 animatedVelocity;
		glm::vec3 initialVelocity;
		glm::vec3 axisOfRotation;
		glm::vec3 rotation;       // 粒子自身的旋转，欧拉角，弧度
		glm::vec3 angularVelocity;
		glm::vec3 startSize;
		glm::vec4 startColor;
		uint32_t  randomSeed;
		uint32_t  parentRandomSeed;
		float     lifetime;       // 剩余寿命
		float     startLifetime;  // 开始时寿命
		int       meshIndex;
		float     emitAccumulator0;
		float     emitAccumulator1;
		uint32_t  flags;

		ParticleSystem* particleSystem;

		Particle(ParticleSystem* ps)
		{
			position = { 0.0f, 0.0f,  0.0f };
			velocity = { 0.0f, 0.0f, -1.0f };
			animatedVelocity = { 0.0f, 0.0f,  0.0f };
			initialVelocity = { 0.0f, 0.0f,  0.0f };
			axisOfRotation = { 0.0f, 0.0f,  0.0f };
			rotation = { 0.0f, 0.0f,  0.0f };
			angularVelocity = { 0.0f, 0.0f,  0.0f };
			startSize = { 0.0f, 0.0f,  0.0f };
			startColor = { 0.0f, 0.0f,  0.0f, 0.0f };
			randomSeed = 0;
			parentRandomSeed = 0;
			lifetime = 0.0f;
			startLifetime = 0.0f;
			meshIndex = 0;
			emitAccumulator0 = 0.0f;
			emitAccumulator1 = 0.0f;
			flags = 0;

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

		void process();
	};

}