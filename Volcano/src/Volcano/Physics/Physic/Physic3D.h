#pragma once

#include "b3_Body.h"
#include "Volcano/Scene/Components.h"

namespace Volcano {

	namespace Utils {

		inline b3_BodyType RigidbodyTypeToBody(RigidbodyComponent::BodyType bodyType)
		{
			switch (bodyType)
			{
			case RigidbodyComponent::BodyType::Static:    return b3_BodyType::e_staticBody;
			case RigidbodyComponent::BodyType::Dynamic:   return b3_BodyType::e_dynamicBody;
			case RigidbodyComponent::BodyType::Kinematic: return b3_BodyType::e_kinematicBody;
			}

			VOL_CORE_ASSERT(false, "Unknown body type");
			return b3_BodyType::e_staticBody;
		}

		inline RigidbodyComponent::BodyType RigidbodyTypeFromBody(b3_BodyType bodyType)
		{
			switch (bodyType)
			{
			case b3_BodyType::e_staticBody:    return RigidbodyComponent::BodyType::Static;
			case b3_BodyType::e_dynamicBody:   return RigidbodyComponent::BodyType::Dynamic;
			case b3_BodyType::e_kinematicBody: return RigidbodyComponent::BodyType::Kinematic;
			}

			VOL_CORE_ASSERT(false, "Unknown body type");
			return RigidbodyComponent::BodyType::Static;
		}

	}
}