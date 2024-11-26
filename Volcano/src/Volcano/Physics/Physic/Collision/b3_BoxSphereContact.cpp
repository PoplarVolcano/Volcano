#include "volpch.h"

#include "Volcano/Physics/Physic/Collision/b3_BoxSphereContact.h"
#include "Volcano/Physics/Physic/b3_BlockAllocator.h"
#include "Volcano/Physics/Physic/Collision/b3_CollideSphere.h"

namespace Volcano {

	b3_Contact* b3_BoxSphereContact::Create(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB, b3_BlockAllocator* allocator)
	{
		void* mem = allocator->Allocate(sizeof(b3_BoxSphereContact));
		return new (mem) b3_BoxSphereContact(fixtureA, fixtureB);
	}

	void b3_BoxSphereContact::Destroy(b3_Contact* contact, b3_BlockAllocator* allocator)
	{
		((b3_BoxSphereContact*)contact)->~b3_BoxSphereContact();
		allocator->Free(contact, sizeof(b3_BoxSphereContact));
	}

	b3_BoxSphereContact::b3_BoxSphereContact(b3_Fixture* fixtureA, b3_Fixture* fixtureB)
		: b3_Contact(fixtureA, 0, fixtureB, 0)
	{
		assert(m_fixtureA->GetType() == b3_Shape::e_box);
		assert(m_fixtureB->GetType() == b3_Shape::e_sphere);
	}

	void b3_BoxSphereContact::Evaluate(b3_Manifold* manifold, const b3_Transform& transformA, const b3_Transform& transformB)
	{
		b3_CollideBoxAndSphere(manifold,
			(b3_BoxShape*)m_fixtureA->GetShape(), transformA,
			(b3_SphereShape*)m_fixtureB->GetShape(), transformB);
	}

}