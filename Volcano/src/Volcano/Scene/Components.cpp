#include "volpch.h"

#include "Components.h"
#include "Volcano/ParticleSystem/ParticleSystem.h"

namespace Volcano {

	ParticleSystemComponent::ParticleSystemComponent()
	{
		particleSystem = std::make_shared<ParticleSystem>();
	}
}