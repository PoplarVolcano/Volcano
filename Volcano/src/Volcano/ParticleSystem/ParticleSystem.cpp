#include "volpch.h"
#include "ParticleSystem.h"

#include "Volcano/Core/Time.h"
#include "Volcano/Scene/Entity.h"
#include "Volcano/Renderer/Renderer2D.h"

namespace Volcano
{
	ParticleSystem::ParticleSystem()
		: m_RandomDevice(std::random_device()), m_mt19937(m_RandomDevice())
	{
		
		isPlaying        = false;
		lastEmissionTime = 0.0f;
		playbackSpeed    = 1.0f;
		playbackTime     = 0.0f;
		
		duration               = 5.0f;
		looping                = true;
		prewarm                = false;
		startDelay1            = 0.0f;
		startDelay2            = 0.0f;
		startDelayType         = 0;
		startLifetime1         = 5.0f;
		startLifetime2         = 5.0f;
		startLifetimeType      = 0;
		startSpeed1            = 5.0f;
		startSpeed2            = 5.0f;
		startSpeedType         = 0;
		useThreeDStartSize     = false;
		threeDStartSize1       = glm::vec3(1.0f);
		threeDStartSize2       = glm::vec3(1.0f);
		startSize1             = 1.0f;
		startSize2             = 1.0f;
		startSizeType          = 0;
		useThreeDStartRotation = false;
		threeDStartRotation1   = glm::vec3(0.0f);
		threeDStartRotation2   = glm::vec3(0.0f);
		startRotation1         = 0.0f;
		startRotation2         = 0.0f;
		startRotationType      = 0;
		flipRotation           = 0.0f;
		startColor1            = glm::vec4(1.0f);
		startColor2            = glm::vec4(1.0f);
		startColorType         = 0;
		simulationSpace        = 0;
		simulationSpeed        = 1.0f;
		isDeltaTimeScaled      = true;
		playOnAwake            = true;
		maxParticles           = 1000;


		emission.enabled              = true;
		emission.rateOverTime1        = 10.0f;
		emission.rateOverTime2        = 10.0f;
		emission.rateOverTimeType     = 0;
		emission.rateOverDistance1    = 0.0f;
		emission.rateOverDistance2    = 0.0f;
		emission.rateOverDistanceType = 0;

		shape.enabled         = true;
		shape.shape           = ParticleSystem_Shape::Shape::Cone;
		shape.angle           = 25.0f;
		shape.radius          = 1.0f;
		shape.radiusThickness = 1.0f;
		shape.arc             = 360.0f;
		shape.boxThickness    = glm::vec3(0.0f);
		shape.mode            = 0;
		shape.length          = 5.0f;
		shape.emitFrom        = ParticleSystem_Shape::EmitFrom::Base;
		shape.texture         = std::string();
		shape.position        = { 0.0f, 0.0f, 0.0f };
		shape.rotation        = { 1.0f, 0.0f, 0.0f, 0.0f };
		shape.scale           = { 1.0f, 1.0f, 1.0f };

		renderer.enabled = true;
		renderer.renderMode = ParticleSystem_Renderer::RenderMode::Billboard;
		renderer.meshes.emplace_back((uint8_t)MeshType::Cube);
		renderer.materialLibraryKey = { "Default", std::hash<std::string> {}("Default") };

		rotationOverLifetime.enabled             = true;
		rotationOverLifetime.separateAxes        = false;
		rotationOverLifetime.angularVelocityType = 0;
		rotationOverLifetime.angularVelocity1    = 0.0f;
		rotationOverLifetime.angularVelocity2    = 45.0f;
		rotationOverLifetime.angularVelocity3D1  = glm::vec3(0.0f);
		rotationOverLifetime.angularVelocity3D2  = glm::vec3(45.0f);


		m_Particles.reserve(maxParticles);

	}

	ParticleSystem::~ParticleSystem()
	{
		Stop();
	}

	ParticleSystem::ParticleSystem(ParticleSystem& other)
	{
		if (other.isPlaying)
			other.Stop();

		isPlaying        = other.isPlaying;
		lastEmissionTime = other.lastEmissionTime;
		playbackSpeed    = other.playbackSpeed;
		playbackTime     = other.playbackTime;

		duration               = other.duration;
		looping                = other.looping;
		prewarm                = other.prewarm;
		startDelay1            = other.startDelay1;
		startDelay2            = other.startDelay2;
		startDelayType         = other.startDelayType;
		startLifetime1         = other.startLifetime1;
		startLifetime2         = other.startLifetime2;
		startLifetimeType      = other.startLifetimeType;
		startSpeed1            = other.startSpeed1;
		startSpeed2            = other.startSpeed2;
		startSpeedType         = other.startSpeedType;
		useThreeDStartSize     = other.useThreeDStartSize;
		startSize1             = other.startSize1;
		startSize2             = other.startSize2;
		threeDStartSize1       = other.threeDStartSize1;
		threeDStartSize2       = other.threeDStartSize2;
		startSizeType          = other.startSizeType;
		useThreeDStartRotation = other.useThreeDStartRotation;
		startRotation1         = other.startRotation1;
		startRotation2         = other.startRotation2;
		threeDStartRotation1   = other.threeDStartRotation1;
		threeDStartRotation2   = other.threeDStartRotation2;
		startRotationType      = other.startRotationType;
		flipRotation           = other.flipRotation;
		startColor1            = other.startColor1;
		startColor2            = other.startColor2;
		startColorType         = other.startColorType;
		simulationSpace        = other.simulationSpace;
		simulationSpeed        = other.simulationSpeed;
		isDeltaTimeScaled      = other.isDeltaTimeScaled;;
		playOnAwake            = other.playOnAwake;
		maxParticles           = other.maxParticles;

		emission.enabled              = other.emission.enabled;
		emission.rateOverTime1        = other.emission.rateOverTime1;
		emission.rateOverTime2        = other.emission.rateOverTime2;
		emission.rateOverTimeType     = other.emission.rateOverTimeType;
		emission.rateOverDistance1    = other.emission.rateOverDistance1;
		emission.rateOverDistance2    = other.emission.rateOverDistance2;
		emission.rateOverDistanceType = other.emission.rateOverDistanceType;
		emission.bursts               = other.emission.bursts;

		shape.enabled         = other.shape.enabled;
		shape.shape           = other.shape.shape;
		shape.angle           = other.shape.angle;
		shape.radius          = other.shape.radius;
		shape.radiusThickness = other.shape.radiusThickness;
		shape.arc             = other.shape.arc;
		shape.boxThickness    = other.shape.boxThickness;
		shape.mode            = other.shape.mode;
		shape.length          = other.shape.length;
		shape.emitFrom        = other.shape.emitFrom;
		shape.texture         = other.shape.texture;
		shape.position        = other.shape.position;
		shape.rotation        = other.shape.rotation;
		shape.scale           = other.shape.scale;

		renderer.enabled            = other.renderer.enabled;
		renderer.renderMode         = other.renderer.renderMode;
		renderer.meshes             = other.renderer.meshes;
		renderer.materialLibraryKey = other.renderer.materialLibraryKey;

		
		rotationOverLifetime.enabled             = other.rotationOverLifetime.enabled;
		rotationOverLifetime.separateAxes        = other.rotationOverLifetime.separateAxes;
		rotationOverLifetime.angularVelocityType = other.rotationOverLifetime.angularVelocityType;
		rotationOverLifetime.angularVelocity1    = other.rotationOverLifetime.angularVelocity1;
		rotationOverLifetime.angularVelocity2    = other.rotationOverLifetime.angularVelocity2;
		rotationOverLifetime.angularVelocity3D1  = other.rotationOverLifetime.angularVelocity3D1;
		rotationOverLifetime.angularVelocity3D2  = other.rotationOverLifetime.angularVelocity3D2;


		m_Particles.reserve(maxParticles);
	}

	void ParticleSystem::Play()
	{
		isPlaying = true;
	}

	void ParticleSystem::Pause()
	{
		isPlaying = false;
	}

	void ParticleSystem::Restart()
	{
		Stop();
		Play();
	}

	void ParticleSystem::Stop()
	{
		isPlaying = false;
		playbackTime = 0.0f;
		lastEmissionTime = 0.0f;

		m_Particles.clear();
	}

	void ParticleSystem::Update(const glm::vec3& position, const glm::vec3& rotation)
	{
		this->cameraPosition = position;
		this->cameraRotation = rotation;

		if (!isPlaying)
		{
			return;
		}

		float deltaTime = Time::GetDeltaTime();

		// 播放状态下，更新当前运行时长，更新现有粒子
		playbackTime += deltaTime;
		UpdateParticles();

		// 播放状态下，发射粒子
		if (emission.enabled)
		{
			// 不循环且当前运行时长时长大于循环时长，则不再产生粒子，等待粒子全部消失后停止播放
			if (!looping && playbackTime > duration)
			{
				if (m_Particles.size() == 0)
				{
					isPlaying = false;
					playbackTime = 0.0f;
				}
			}
			else
			{
				EmissionParticles();
			}
		}
	}

	void ParticleSystem::Render()
	{
		RenderParticles();

	}

	uint32_t ParticleSystem::CreateParticle()
	{
		m_Particles.emplace_back(this);
		return (uint32_t)(m_Particles.size() - 1);
	}

	void ParticleSystem::UpdateParticles()
	{
		// 从后往前遍历，因为交换删除会改变数组长度
		for (int i = (int)m_Particles.size() - 1; i >= 0; --i)
		{
			Particle& particle = m_Particles[i];
			particle.lifetime -= Time::GetDeltaTime();
			particle.Update();

			if (particle.lifetime <= 0.0f)
			{
				// 将当前死亡粒子与数组最后一个粒子（已更新）交换
				// 然后弹出最后一个（此时最后一个就是刚死亡的粒子，或者原本就是它自己）
				std::swap(m_Particles[i], m_Particles.back());
				m_Particles.pop_back();
			}
		}
	}

	void ParticleSystem::RenderParticles()
	{
		for (int i = (int)m_Particles.size() - 1; i >= 0; --i)
		{
			m_Particles[i].Render();
		}
	}

}