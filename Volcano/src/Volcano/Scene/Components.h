#pragma once

#include "Volcano/Scene/Components/BasicComponent.h"
#include "Volcano/Scene/Components/CameraComponent.h"
#include "Volcano/Scene/Components/CircleRendererComponent.h"
#include "Volcano/Scene/Components/MeshComponent.h"
#include "Volcano/Scene/Components/MeshRendererComponent.h"
#include "Volcano/Scene/Components/LightComponent.h"
#include "Volcano/Scene/Components/SkyboxComponent.h"
#include "Volcano/Scene/Components/HDRComponent.h"
#include "Volcano/Scene/Components/ParticleSystemComponent.h"
#include "Volcano/Scene/Components/ScriptComponent.h"
#include "Volcano/Scene/Components/Rigidbody2DComponent.h"
#include "Volcano/Scene/Components/BoxCollider2DComponent.h"
#include "Volcano/Scene/Components/CircleCollider2DComponent.h"

namespace Volcano {


	template<typename... Component>
	struct ComponentGroup
	{
	};

	// (except IDComponent and TagComponent)
	using AllComponents = ComponentGroup<
		TransformComponent,
		CameraComponent,
		LightComponent,
		CircleRendererComponent,
		MeshComponent,
		MeshRendererComponent,
		SkyboxComponent,
		HDRComponent,
		ParticleSystemComponent,
		ScriptComponent,
		Rigidbody2DComponent,
		BoxCollider2DComponent,
		CircleCollider2DComponent>;

}