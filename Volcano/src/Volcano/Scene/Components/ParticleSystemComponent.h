#pragma once

#include "Volcano/ParticleSystem/ParticleSystem.h"

namespace Volcano
{

	struct ParticleSystemComponent
	{
		bool enabled = true;
		Ref<ParticleSystem> particleSystem = std::make_shared<ParticleSystem>();

		ParticleSystemComponent() = default;
		ParticleSystemComponent(const ParticleSystemComponent& other)
			: enabled(other.enabled), particleSystem(std::make_shared<ParticleSystem>(*other.particleSystem))
		{}
	};

}