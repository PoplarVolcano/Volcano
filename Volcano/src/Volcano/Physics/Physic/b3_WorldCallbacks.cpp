#include "volpch.h"

#include "Volcano/Scripting/ScriptEngine.h"
#include "b3_WorldCallbacks.h"
#include "b3_Fixture.h"
#include "b3_Contact.h"

namespace Volcano {

	bool b3_ContactFilter::ShouldCollide(b3_Fixture* fixtureA, b3_Fixture* fixtureB)
	{
		const b3_Filter& filterA = fixtureA->GetFilterData();
		const b3_Filter& filterB = fixtureB->GetFilterData();

		if (filterA.groupIndex == filterB.groupIndex && filterA.groupIndex != 0)
		{
			return filterA.groupIndex > 0;
		}

		bool collide = (filterA.maskBits & filterB.categoryBits) != 0 && (filterA.categoryBits & filterB.maskBits) != 0;
		return collide;
	}

	void b3_ContactListener::BeginContact(b3_Contact* contact)
	{
		VOL_TRACE("Begin collide!!!");
		(void)(contact); /* not used */

		Scene* scene = ScriptEngine::GetSceneContext();
		if (scene)
		{
			UUID srcUUID = contact->GetFixtureB()->GetBody()->GetUserData().pointer;
			Ref<Entity> entity = ScriptEngine::GetSceneContext()->GetEntityByUUID(srcUUID);
			if (entity && entity->HasComponent<ScriptComponent>() && entity->GetComponent<ScriptComponent>().enabled)
			{
				auto instance = ScriptEngine::GetEntityScriptInstance(srcUUID);
				if (instance)
				{
					UUID dstUUID = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
					instance->InvokeOnTriggerEnter(ScriptEngine::CreateInstance(ScriptEngine::GetColliderClass()->GetClass(), dstUUID));
				}
			}
		}
	}

}