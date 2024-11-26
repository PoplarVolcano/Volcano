#pragma once

#include "b3_Common.h"
#include "b3_Math.h"

namespace Volcano {

	class b3_Joint;
	class b3_Fixture;
	class b3_Contact;
	struct b3_Manifold;


	// 当Joints和fixtures的相关body被销毁时，它们也会被破坏。执行(Implement)此侦听器，以便可以取消(nullify)对这些Joints和shapes的引用。
	class b3_DestructionListener
	{
	public:
		virtual ~b3_DestructionListener() {}
		virtual void SayGoodbye(b3_Joint* joint) = 0;// 当任何关节因其所属body之一的销毁而即将被销毁时，会调用此功能。
		virtual void SayGoodbye(b3_Fixture* fixture) = 0; // 当任何fixture由于其所属body的销毁而即将被销毁时调用。
	};

	// 实现此类用以提供碰撞过滤。果你想对contact创建进行更精细的控制(finer control)，你可以实现此类。
	class b3_ContactFilter
	{
	public:
		virtual ~b3_ContactFilter() {}
		// 如果应在这两个shape之间执行contact计算，则返回true。
		// @warning 出于性能原因，这仅在AABB开始重叠时调用。 for performance reasons this is only called when the AABBs begin to overlap.
		virtual bool ShouldCollide(b3_Fixture* fixtureA, b3_Fixture* fixtureB);
	};

	// Contact冲量。使用冲量(Impulses)代替力，因为刚体碰撞的子步(sub-step)力可能接近无穷大。这些与b3_Manifold中的contact点一对一匹配。
	struct b3_ContactImpulse
	{
		float normalImpulses[b3_MaxManifoldPoints];  // 碰撞合力产生的冲量
		float tangentImpulses[b3_MaxManifoldPoints]; // 模拟切线方向摩擦力所产生的冲量
		int count;
	};

	/*
	    实现此类以获取contact信息。你可以将这些结果用于声音和游戏逻辑等方面。您还可以通过在时间步长后遍历contact列表来获取contact结果。
		然而，你可能会错过一些contact，因为连续的物理会导致子步进sub-stepping(sub-stepping)。 
        此外，您可能会在一个时间步内收到同一contact的多个回调。 
        你应该努力(strive)使你的回调高效，因为每个时间步可能有很多回调。 
        @warning 您无法在这些回调函数中创建/销毁Box3D实体。
	*/
	class b3_ContactListener
	{
	public:
		virtual ~b3_ContactListener() {}
		// 两个fixtures开始碰撞时调用.  not used
		virtual void BeginContact(b3_Contact* contact);
		// 两个fixtures停止碰撞时调用.  not used
		virtual void EndContact(b3_Contact* contact) { VOL_TRACE("End collide!!!"); (void)(contact); /* not used */ }

		/*
		    两个fixtures碰撞期间调用的。这允许您在contact进入求解器(solver)之前对其进行检查(inspect)。
			如果你很谨慎，你可以修改contact manifold（例如禁用contact）。 
            提供了上一帧的Manifold(oldManifold)，以便您可以检测(detect)到交点的变化。 
            注：仅awake的body会触发。 
            注：即使交点的数量为零也会触发。 
            注：传感器不会触发。 
            注：如果将contact点数量设置为零，则不会收到EndContact回调。但是，下一帧您可能会收到BeginContact回拨。

			not used
		*/
		virtual void PreSolve(b3_Contact* contact, const b3_Manifold* oldManifold)
		{
			VOL_TRACE("Colliding!!!");
			(void)(contact); /* not used */
			(void)(oldManifold); /* not used */
		}

		/*
		    这使您可以在求解器(solver)完成后检查(inspect)接触。这对于检查冲量很有用。 
            注：contact manifold不包括冲击冲量的时间，如果子步骤(sub-step)很小，冲击脉冲可以任意(arbitrarily)大。
			因此，冲量在单独的数据结构中明确(explicitly)提供。 
            注意：这仅适用于touching、solid和awake的contacts。
		*/
		virtual void PostSolve(b3_Contact* contact, const b3_ContactImpulse* impulse)
		{
			(void)(contact); // not used
			(void)(impulse); // not used
		}
	};

	// AABB查询的回调类  Callback class for AABB queries.  See b3_World::Query
	class b3_QueryCallback
	{
	public:
		virtual ~b3_QueryCallback() {}

		// 为查询AABB中找到的每个fixture调用callback。
		// @return false to terminate the query.
		virtual bool ReportFixture(b3_Fixture* fixture) = 0;
	};

	// ray casts的回调类  Callback class for ray casts.   See b3_World::RayCast
	class b3_RayCastCallback
	{
	public:
		virtual ~b3_RayCastCallback() {}

		
		// 为查询(query)中找到的每个fixture调用。通过返回一个浮点数来控制如何处理ray cast:
		// return -1：       忽略此fixture，继续 
		// return  0：       终止ray cast 
		// return  fraction：光线在这个点碰撞 
		// return  1：       光线没有碰撞，继续 
		// @param fixture  被射线击中的fixture 
		// @param point    初始交点 
		// @param normal   交点处的法向量 
		// @param fraction 交点处沿射线的分数 
		// @return -1表示过滤，0表示终止，fraction表示光线最近的碰撞点，1表示继续
		virtual float ReportFixture(b3_Fixture* fixture, const glm::vec3& point, const glm::vec3& normal, float fraction) = 0;
	};

}