#include "volpch.h"
#include "Entity.h"

namespace Volcano {

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
		
	}
}
