#include "volpch.h"
#include "Scene.h"

#include "Volcano/Scene/Entity.h"

namespace Volcano
{
	// 将源注册表下实体复制到目标注册表，Map以UUID作为标记获取新实体
	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<Volcano::UUID, Ref<Entity>>& entityIDMap)
	{
		// 这个lambda会递归调用
		// [&]表示引用传递方式捕捉所有父作用域的变量（包括this）
		// 隐式引用捕获dst、src或者"解开的Component包引用"，下面的Component是指具体的单个组件
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					Volcano::UUID& id = src.get<IDComponent>(srcEntity).ID;
					VOL_CORE_ASSERT(entityIDMap.find(id) != entityIDMap.end());
					entt::entity dstEntity = entityIDMap.at(id)->GetEntityHandle();
					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}

			}(), ...);// 这三个点应该是解Component包
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<Volcano::UUID, Ref<Entity>>& entityIDMap)
	{
		CopyComponent<Component...>(dst, src, entityIDMap);
	}

	// 如果源实体存在则把对应组件复制到目标实体
	template<typename... Component>
	static void CopyComponentIfExists(Ref<Entity> dst, Ref<Entity> src)
	{
		([&]()
			{
				if (src->HasComponent<Component>())
					dst->AddOrReplaceComponent<Component>(src->GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Ref<Entity> dst, Ref<Entity> src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	Ref<Entity> Scene::FindEntityByName(std::string name, std::vector<Ref<Entity>>& entityList)
	{
		for (auto& entity : entityList)
		{
			if (entity && entity->GetName() == name)
			{
				return entity;
			}
		}
		return nullptr;
	}

	std::string Scene::NewName(std::vector<Ref<Entity>>& entityList, std::string name)
	{
		std::string newName = name;
		int i = 0;
		while (FindEntityByName(newName, entityList) != nullptr)
		{
			newName = name + "(" + std::to_string(i) + ")";
			i++;
		}

		return newName;
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		// 创建新场景，为新场景创建和旧场景同名和uuid的实体，并用map存入（旧实体的uuid对应新实体）的关系
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		auto& entityIDMap = newScene->GetEntityIDMap();
		auto& entityEnttMap = newScene->GetEntityEnttMap();
		auto& entityList = newScene->GetEntityList();

		auto& entityChildrenList = other->GetEntityList();
		for (int i = 0; i != entityChildrenList.size(); i++)
		{
			auto& entityChild = entityChildrenList[i];
			newScene->DuplicateEntity(entityChild, nullptr, entityChild->GetUUID(), i);
		}

		newScene->m_MaterialLibrary.Copy(other->m_MaterialLibrary);

		newScene->m_SSAO         = other->m_SSAO;
		newScene->m_HDREnabled   = other->m_HDREnabled;
		newScene->m_Exposure	 = other->m_Exposure;
		newScene->m_BloomEnabled = other->m_BloomEnabled;

		return newScene;
	}

	Ref<Entity> Scene::Find(std::string name, std::vector<Ref<Entity>>& entityList)
	{
		// 将name按路径分割，可根据路径查找实体
		std::vector<std::string> result;
		std::filesystem::path fs_path(name);
		for (auto& piece : fs_path)
			result.push_back(piece.string());

		auto currentName = result.begin();
		std::stack<Ref<Entity>> entityStack;
		for (auto& entity : entityList)
		{
			entityStack.push(entity);
		}

		while (!entityStack.empty())
		{
			Ref<Entity> entity = entityStack.top();
			entityStack.pop();

			if (entity->GetName() == *currentName)
				if (currentName == result.end() - 1)
					return entity;
				else
					currentName++;

			for (auto& child : entity->GetEntityChildrenList())
			{
				entityStack.push(child);
			}
		}

		return nullptr;
	}

	void Scene::RemoveEntityFromList(Ref<Entity> entity, std::vector<Ref<Entity>>& entityList)
	{
		auto it = std::find_if(entityList.begin(), entityList.end(), [&](Ref<Entity>& entityTemp) { return entityTemp->GetUUID() == entity->GetUUID(); });
		if (it != entityList.end())
			entityList.erase(it);
	}

	void Scene::MoveEntityToEntity(Ref<Entity> entity, Entity* parent, int index)
	{
		if (entity == nullptr || parent == nullptr || index > (int)parent->GetEntityChildrenList().size() || index < -1)
			return;

		if (entity->GetScene() == parent->GetScene())
		{
			// 如果parent是entity的父节点，则仅移动
			Entity* entityParent = entity->GetEntityParent();
			if (entityParent == parent)
			{
				auto& entityParentChildrenList = entityParent->GetEntityChildrenList();

				auto it = std::find(entityParentChildrenList.begin(), entityParentChildrenList.end(), entity);
				if (it == entityParentChildrenList.end())
				{
					if (index == -1)
						entityParentChildrenList.push_back(entity);
					else
						entityParentChildrenList.insert(entityParentChildrenList.begin() + index, entity);
					return;
				}

				size_t fromIndex = it - entityParentChildrenList.begin();

				if (index == fromIndex || index == fromIndex + 1)
					return;   // 位置没变

				Ref<Entity> entityTemp = std::move(*it);
				entityParentChildrenList.erase(it);

				// 修正目标索引，erase 后，fromIndex 之后的元素都左移了一位
				if (index != -1 && fromIndex < index)
					index--;

				if (index == -1)
					entityParentChildrenList.push_back(entityTemp);
				else
				    entityParentChildrenList.insert(entityParentChildrenList.begin() + index, std::move(entityTemp));

			}
			else
			{
				// 从entity的父节点删除entity
				if (entityParent != nullptr)
					Scene::RemoveEntityFromList(entity, entity->GetEntityParent()->GetEntityChildrenList());
				else
					Scene::RemoveEntityFromList(entity, entity->GetScene()->GetEntityList());

				auto& parentChildrenList = parent->GetEntityChildrenList();
				entity->SetName(Scene::NewName(parentChildrenList, entity->GetName()));
				if (index == -1)
					parentChildrenList.push_back(entity);
				else
					parentChildrenList.insert(parentChildrenList.begin() + index, entity);

				entity->SetEntityParent(parent);
			}
		}
		else
		{
			// 跨Scene移动实体时，entityID不变，便于跨Scene追踪Entity，这里是move函数，如果要复制多个实体应该找prefab或copy
			if (parent->GetScene()->DuplicateEntity(entity, parent, entity->GetUUID(), index) != nullptr)
			{
				entity->GetScene()->DestroyEntity(entity);
			}
		}
	}

	void Scene::MoveEntityToScene(Ref<Entity> entity, Scene* targetScene, int index)
	{
		if (entity == nullptr || targetScene == nullptr)
		{
			if (entity->GetScene() == targetScene)
			{
				entity->GetScene()->MoveEntity(entity, index);
				return;
			}
			else
			{
				// 跨Scene移动实体时，entityID不变，便于跨Scene追踪Entity，这里是move函数，如果要复制多个实体应该找prefab或copy
				if (targetScene->DuplicateEntity(entity, nullptr, entity->GetUUID(), index) != nullptr)
				{
					entity->GetScene()->DestroyEntity(entity);
				}
			}
		}

	}

	void Scene::MoveEntity(Ref<Entity> entity, int index)
	{
		if (entity == nullptr || index > (int)m_EntityList.size() || index < -1)
			return;

		if (entity->GetScene() == this)
		{
			if (Entity* parent = entity->GetEntityParent())
			{
				Scene::RemoveEntityFromList(entity, parent->GetEntityChildrenList());
				entity->SetEntityParent(nullptr);
				entity->SetName(Scene::NewName(m_EntityList, entity->GetName()));
				if (index == -1)
				    m_EntityList.push_back(entity);
				else
					m_EntityList.insert(m_EntityList.begin() + index, entity);
			}
			else
			{
				auto it = std::find(m_EntityList.begin(), m_EntityList.end(), entity);
				if (it == m_EntityList.end())
				{
					if (index == -1)
						m_EntityList.push_back(entity);
					else
						m_EntityList.insert(m_EntityList.begin() + index, entity);
					return;
				}

				size_t fromIndex = it - m_EntityList.begin();

				if (index == fromIndex || index == fromIndex + 1)
					return;   // 位置没变

				Ref<Entity> entityTemp = std::move(*it);
				m_EntityList.erase(it);

				// 修正目标索引，erase 后，fromIndex 之后的元素都左移了一位
				if (index != -1 && fromIndex < index)
					index--;

				if (index == -1)
					m_EntityList.push_back(entityTemp);
				else
				    m_EntityList.insert(m_EntityList.begin() + index, std::move(entityTemp));

			}

		}
		else
		{
			// 跨Scene移动实体时，entityID不变，便于跨Scene追踪Entity，这里是move函数，如果要复制多个实体应该找prefab或copy
			if (DuplicateEntity(entity, nullptr, entity->GetUUID(), index) != nullptr)
			{
				entity->GetScene()->DestroyEntity(entity);
			}
		}

	}

	// 遍历Active的entity，注意Scene.Destroy方法会把entity的父节点的子节点列表清除， TraverseEntity不会继续递归
	void Scene::TraverseEntity(std::vector<Ref<Entity>>& entityList, const std::function<bool(Ref<Entity>)>& function)
	{
		// 将entityList所有实体入栈，然后逐个出栈，如果实体激活，则对实体执行function，然后将它的子节点入栈，直到所有实体执行过function
		// 因为function返回bool，如果执行完function后需要退出遍历，则返回false，然后退出遍历
		std::stack<Ref<Entity>> entityStack;
		for (auto& entity : entityList)
		{
			entityStack.push(entity);
		}

		while (!entityStack.empty())
		{
			Ref<Entity> entity = std::move(entityStack.top());
			entityStack.pop();

			if (entity->GetActive())
			{
				if (!function(entity))
					return;

				for (auto& child : entity->GetEntityChildrenList())
				{
					entityStack.push(child);
				}
			}
		}
	}


	Ref<Entity> Scene::CreateEntity(const std::string& name, Entity* parent, int index)
	{
		return CreateEntityWithUUID(Volcano::UUID(), name, parent, index);
	}

	Ref<Entity> Scene::CreateEntityWithUUID(Volcano::UUID uuid, const std::string& name, Entity* parent, int index)
	{
		Ref<Entity> entity;

		if (parent != nullptr)
		{
			entity = parent->AddEntityChild(uuid, name, index);
			m_EntityIDMap[entity->GetUUID()] = entity;
			m_EntityEnttMap[entity->GetEntityHandle()] = entity;
			return entity;
		}
		else
		{
			if (index > (int)m_EntityList.size() || index < -1)
				return nullptr;
			std::string newName = NewName(m_EntityList, name);
			entity = Entity::Create(this, uuid, newName);
			m_EntityIDMap[entity->GetUUID()] = entity;
			m_EntityEnttMap[entity->GetEntityHandle()] = entity;
			if (index == -1)
				m_EntityList.push_back(entity);
			else
				m_EntityList.insert(m_EntityList.begin() + index, entity);

			return entity;
		}
	}

	Ref<Entity> Scene::GetEntityByUUID(Volcano::UUID uuid)
	{
		if (m_EntityIDMap.find(uuid) != m_EntityIDMap.end())
			return m_EntityIDMap[uuid];
		return {};
	}


	Ref<Entity> Scene::DuplicateEntity(Ref<Entity> entity, Entity* parent, Volcano::UUID id, int index)
	{
		std::string name = entity->GetName();
		Ref<Entity> newEntity = CreateEntityWithUUID(id, name, parent, index);
		if (newEntity == nullptr)
			return nullptr;

		newEntity->SetActive(entity->GetActive());
		CopyComponentIfExists(AllComponents{}, newEntity, entity);

		auto& entityChildrenList = entity->GetEntityChildrenList();
		for (int i = 0; i != entityChildrenList.size(); i++)
		{
			auto& entityChild = entityChildrenList[i];
			// 如果输入的id和entity的ID不一致，说明这不是移动，而是复制新实体，所以子节点的复制也要用新id
			if (id == entity->GetUUID())
			{
				DuplicateEntity(entityChild, newEntity.get(), entityChild->GetUUID(), i);
			}
			else
			{
				DuplicateEntity(entityChild, newEntity.get(), UUID(), i);
			}
		}

		return newEntity;
	}

	void Scene::DestroyEntityChild(Ref<Entity> entity)
	{
		auto& entityChildren = entity->GetEntityChildrenList();
		while (!entityChildren.empty())
		{
			auto it = entityChildren.end() - 1;
			DestroyEntityChild(*it);
			m_EntityIDMap.erase((*it)->GetUUID());
			m_EntityEnttMap.erase((*it)->GetEntityHandle());
			entityChildren.erase(it);
			m_Registry.destroy((*it)->GetEntityHandle());
		}
	}

	void Scene::DestroyEntity(Ref<Entity> entity)
	{
		// 清理所有子节点
		DestroyEntityChild(entity);

		m_EntityIDMap.erase(entity->GetUUID());
		m_EntityEnttMap.erase(entity->GetEntityHandle());
		// 从父节点中移除自己
		Entity* entityParent = entity->GetEntityParent();
		if (entityParent == nullptr)
		{
			Scene::RemoveEntityFromList(entity, m_EntityList);
		}
		else
		{
			Scene::RemoveEntityFromList(entity, entityParent->GetEntityChildrenList());
		}
		m_Registry.destroy(entity->GetEntityHandle());
	}

}