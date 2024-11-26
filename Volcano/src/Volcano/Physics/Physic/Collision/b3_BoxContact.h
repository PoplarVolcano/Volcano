#pragma once

#include "Volcano/Physics/Physic/b3_Contact.h"

namespace Volcano {

	class b3_BlockAllocator;

	class b3_BoxContact : public b3_Contact
	{
	public:
		static b3_Contact* Create(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB, b3_BlockAllocator* allocator);
		static void Destroy(b3_Contact* contact, b3_BlockAllocator* allocator);

		b3_BoxContact(b3_Fixture* fixtureA, b3_Fixture* fixtureB);
		~b3_BoxContact() {}

		void Evaluate(b3_Manifold* manifold, const b3_Transform& transformA, const b3_Transform& transformB) override;
	};

}