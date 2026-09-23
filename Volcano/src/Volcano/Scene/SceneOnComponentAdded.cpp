#include "volpch.h"
#include "Scene.h"

namespace Volcano
{
	template<typename T>
	void Scene::OnComponentAdded(Entity& entity, T& component)
	{
		// 静态断言
		static_assert(sizeof(T) == 0);
	}

	template<>
	VOL_API void Scene::OnComponentAdded<IDComponent>(Entity& entity, IDComponent& component)
	{
	}

	template<>
	VOL_API void Scene::OnComponentAdded<TagComponent>(Entity& entity, TagComponent& component)
	{

	}

	template<>
	VOL_API void Scene::OnComponentAdded<TransformComponent>(Entity& entity, TransformComponent& component)
	{

	}

	template<>
	VOL_API void Scene::OnComponentAdded<CameraComponent>(Entity& entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	VOL_API void Scene::OnComponentAdded<CircleRendererComponent>(Entity& entity, CircleRendererComponent& component)
	{

	}

	template<>
	VOL_API void Scene::OnComponentAdded<MeshComponent>(Entity& entity, MeshComponent& component)
	{

	}

	template<>
	VOL_API void Scene::OnComponentAdded<MeshRendererComponent>(Entity& entity, MeshRendererComponent& component)
	{

	}

	template<>
	VOL_API void Scene::OnComponentAdded<LightComponent>(Entity& entity, LightComponent& component)
	{

	}

	template<>
	VOL_API void Scene::OnComponentAdded<SkyboxComponent>(Entity& entity, SkyboxComponent& component)
	{
		
	}

	template<>
	VOL_API void Scene::OnComponentAdded<HDRComponent>(Entity& entity, HDRComponent& component)
	{

	}

	template<>
	VOL_API void Scene::OnComponentAdded<ParticleSystemComponent>(Entity& entity, ParticleSystemComponent& component)
	{
		component.particleSystem->entity = &entity;
	}
	
	template<>
	VOL_API void Scene::OnComponentAdded<ScriptComponent>(Entity& entity, ScriptComponent& component)
	{

	}

	template<>
	VOL_API void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity& entity, Rigidbody2DComponent& component)
	{

	}

	template<>
	VOL_API void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity& entity, BoxCollider2DComponent& component)
	{

	}

	template<>
	VOL_API void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity& entity, CircleCollider2DComponent& component)
	{

	}
}