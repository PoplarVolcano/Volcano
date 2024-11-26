#include "volpch.h"

#include "Volcano/Physics/Physic/Collision/b3_BoxContact.h"
#include "Volcano/Physics/Physic/b3_BlockAllocator.h"
#include "Volcano/Physics/Physic/Collision/b3_CollideSphere.h"

namespace Volcano {

	b3_Contact* b3_BoxContact::Create(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB, b3_BlockAllocator* allocator)
	{
		void* mem = allocator->Allocate(sizeof(b3_BoxContact));
		return new (mem) b3_BoxContact(fixtureA, fixtureB);
	}

	void b3_BoxContact::Destroy(b3_Contact* contact, b3_BlockAllocator* allocator)
	{
		((b3_BoxContact*)contact)->~b3_BoxContact();
		allocator->Free(contact, sizeof(b3_BoxContact));
	}

	b3_BoxContact::b3_BoxContact(b3_Fixture* fixtureA, b3_Fixture* fixtureB)
		: b3_Contact(fixtureA, 0, fixtureB, 0)
	{
		assert(m_fixtureA->GetType() == b3_Shape::e_box);
		assert(m_fixtureB->GetType() == b3_Shape::e_box);
	}

	void b3_BoxContact::Evaluate(b3_Manifold* manifold, const b3_Transform& transformA, const b3_Transform& transformB)
	{
		b3_CollideBoxs(manifold,
			(b3_BoxShape*)m_fixtureA->GetShape(), transformA,
			(b3_BoxShape*)m_fixtureB->GetShape(), transformB);
	}

}