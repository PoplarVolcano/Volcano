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
	///��ɢ����
	/// </summary>
	/// <param name="particleProps"></param>
	void ParticleSystem::Emit(const ParticleProps& particleProps)
	{
		//�����ӳ���ȡ��һ��
		Particle& particle = m_ParticlePool[m_PoolIndex];
		//����
		particle.Active = true;
		//����λ��
		particle.Position = particleProps.Position;
		//������ת����� [0,2��]
		particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

		// Velocity �����ٶ�
		particle.Velocity = particleProps.Velocity;
		particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
		particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);

		// Color ������ɫ
		particle.ColorBegin = particleProps.ColorBegin;
		particle.ColorEnd = particleProps.ColorEnd;

		// Size ���ô�С
		particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
		particle.SizeEnd = particleProps.SizeEnd;

		// Life ������������
		particle.LifeTime = particleProps.LifeTime;
		particle.LifeRemaining = particleProps.LifeTime;
		//������һ��ȡģ����֤����0
		m_PoolIndex = (m_PoolIndex - 1) % m_ParticlePool.size();
	}

	void ParticleSystem::OnUpdate(Timestep ts)
	{
		//���³��ӵ�ÿ��Ԫ��
		for (auto& particle : m_ParticlePool)
		{
			//���û���ֱ������
			if (!particle.Active)
				continue;
			//����������ڵ�ͷ�ˣ�ֱ������δ����
			if (particle.LifeRemaining <= 0.0f)
			{
				particle.Active = false;
				continue;
			}
			//ÿ��ˢ���������ڼ���
			particle.LifeRemaining -= ts;
			//λ�ø���
			particle.Position += particle.Velocity * (float)ts;
			//��ת���£��Զ���ת��
			particle.Rotation += 0.01f * ts;
		}
	}

	void ParticleSystem::OnRender()
	{
		//ȡ������
		for (auto& particle : m_ParticlePool)
		{
			//���û�м���ֱ�Ӳ�����
			if (!particle.Active)
				continue;
			//��ȡlife
			float life = particle.LifeRemaining / particle.LifeTime;
			//����life����Color�任
			glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);
			//����life����͸����
			color.a = color.a * life;
			//����life���ɴ�С
			float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);
			//��Ⱦ����
			Renderer2D::DrawRotatedQuad(particle.Position, { size, size }, particle.Rotation, color);
		}
	}
}