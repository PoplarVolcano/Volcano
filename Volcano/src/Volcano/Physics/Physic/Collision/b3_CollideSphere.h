#pragma once

#include "Volcano/Physics/Physic/b3_Math.h"

namespace Volcano {

	struct b3_Manifold;
	class b3_SphereShape;
	class b3_BoxShape;

	// sphereAºÍÊÇ·ñsphereBÅö×²
	void b3_CollideSpheres(b3_Manifold* manifold,
		const b3_SphereShape* sphereA, const b3_Transform& transformA,
		const b3_SphereShape* sphereB, const b3_Transform& transformB);

	
	void b3_CollideBoxAndSphere(
		b3_Manifold* manifold,
		const b3_BoxShape* boxA, const b3_Transform& transformA,
		const b3_SphereShape* sphereB, const b3_Transform& transformB);

	void b3_CollideBoxs(b3_Manifold* manifold,
		const b3_BoxShape* polygonA, const b3_Transform& transformA,
		const b3_BoxShape* polygonB, const b3_Transform& transformB);

}