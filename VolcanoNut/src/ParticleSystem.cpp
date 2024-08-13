#include "ParticleSystem.h"

#include "Random.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

namespace Volcano {
	ParticleSystem::ParticleSystem(uint32_t maxParticles)
		: m_PoolIndex(maxParticles - 1)
	{
		m_ParticlePool.resize(maxParticles);
	}

	/// <summary>
	///发散粒子
	/// </summary>
	/// <param name="particleProps"></param>
	void ParticleSystem::Emit(const ParticleProps& particleProps)
	{
		//从粒子池中取出一个
		Particle& particle = m_ParticlePool[m_PoolIndex];
		//激活
		particle.Active = true;
		//设置位置
		particle.Position = particleProps.Position;
		//设置旋转随机数 [0,2π]
		particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

		// Velocity 设置速度
		particle.Velocity = particleProps.Velocity;
		particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
		particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);

		// Color 设置颜色
		particle.ColorBegin = particleProps.ColorBegin;
		particle.ColorEnd = particleProps.ColorEnd;

		// Size 设置大小
		particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
		particle.SizeEnd = particleProps.SizeEnd;

		// Life 设置生命周期
		particle.LifeTime = particleProps.LifeTime;
		particle.LifeRemaining = particleProps.LifeTime;
		//索引减一后取模，保证大于0
		m_PoolIndex = (m_PoolIndex - 1) % m_ParticlePool.size();
	}

	void ParticleSystem::OnUpdate(Timestep ts)
	{
		//更新池子的每个元素
		for (auto& particle : m_ParticlePool)
		{
			//如果没激活，直接跳过
			if (!particle.Active)
				continue;
			//如果生命周期到头了，直接设置未激活
			if (particle.LifeRemaining <= 0.0f)
			{
				particle.Active = false;
				continue;
			}
			//每次刷新生命周期减少
			particle.LifeRemaining -= ts;
			//位置更新
			particle.Position += particle.Velocity * (float)ts;
			//旋转更新（自动旋转）
			particle.Rotation += 0.01f * ts;
		}
	}

	void ParticleSystem::OnRender()
	{
		//取出粒子
		for (auto& particle : m_ParticlePool)
		{
			//如果没有激活直接不处理
			if (!particle.Active)
				continue;
			//获取life
			float life = particle.LifeRemaining / particle.LifeTime;
			//根据life过渡Color变换
			glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);
			//根据life过渡透明度
			color.a = color.a * life;
			//根据life过渡大小
			float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);
			//渲染粒子
			Renderer2D::DrawRotatedQuad(particle.Position, { size, size }, particle.Rotation, color);
		}
	}
}