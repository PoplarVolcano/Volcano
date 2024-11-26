#pragma once

#include "b3_Common.h"
#include "b3_Math.h"

namespace Volcano {

	class b3_Joint;
	class b3_Fixture;
	class b3_Contact;
	struct b3_Manifold;


	// ��Joints��fixtures�����body������ʱ������Ҳ�ᱻ�ƻ���ִ��(Implement)�����������Ա����ȡ��(nullify)����ЩJoints��shapes�����á�
	class b3_DestructionListener
	{
	public:
		virtual ~b3_DestructionListener() {}
		virtual void SayGoodbye(b3_Joint* joint) = 0;// ���κιؽ���������body֮һ�����ٶ�����������ʱ������ô˹��ܡ�
		virtual void SayGoodbye(b3_Fixture* fixture) = 0; // ���κ�fixture����������body�����ٶ�����������ʱ���á�
	};

	// ʵ�ִ��������ṩ��ײ���ˡ��������contact�������и���ϸ�Ŀ���(finer control)�������ʵ�ִ��ࡣ
	class b3_ContactFilter
	{
	public:
		virtual ~b3_ContactFilter() {}
		// ���Ӧ��������shape֮��ִ��contact���㣬�򷵻�true��
		// @warning ��������ԭ�������AABB��ʼ�ص�ʱ���á� for performance reasons this is only called when the AABBs begin to overlap.
		virtual bool ShouldCollide(b3_Fixture* fixtureA, b3_Fixture* fixtureB);
	};

	// Contact������ʹ�ó���(Impulses)����������Ϊ������ײ���Ӳ�(sub-step)�����ܽӽ��������Щ��b3_Manifold�е�contact��һ��һƥ�䡣
	struct b3_ContactImpulse
	{
		float normalImpulses[b3_MaxManifoldPoints];  // ��ײ���������ĳ���
		float tangentImpulses[b3_MaxManifoldPoints]; // ģ�����߷���Ħ�����������ĳ���
		int count;
	};

	/*
	    ʵ�ִ����Ի�ȡcontact��Ϣ������Խ���Щ���������������Ϸ�߼��ȷ��档��������ͨ����ʱ�䲽�������contact�б�����ȡcontact�����
		Ȼ��������ܻ���һЩcontact����Ϊ����������ᵼ���Ӳ���sub-stepping(sub-stepping)�� 
        ���⣬�����ܻ���һ��ʱ�䲽���յ�ͬһcontact�Ķ���ص��� 
        ��Ӧ��Ŭ��(strive)ʹ��Ļص���Ч����Ϊÿ��ʱ�䲽�����кܶ�ص��� 
        @warning ���޷�����Щ�ص������д���/����Box3Dʵ�塣
	*/
	class b3_ContactListener
	{
	public:
		virtual ~b3_ContactListener() {}
		// ����fixtures��ʼ��ײʱ����.  not used
		virtual void BeginContact(b3_Contact* contact);
		// ����fixturesֹͣ��ײʱ����.  not used
		virtual void EndContact(b3_Contact* contact) { VOL_TRACE("End collide!!!"); (void)(contact); /* not used */ }

		/*
		    ����fixtures��ײ�ڼ���õġ�����������contact���������(solver)֮ǰ������м��(inspect)��
			�����ܽ�����������޸�contact manifold���������contact���� 
            �ṩ����һ֡��Manifold(oldManifold)���Ա������Լ��(detect)������ı仯�� 
            ע����awake��body�ᴥ���� 
            ע����ʹ���������Ϊ��Ҳ�ᴥ���� 
            ע�����������ᴥ���� 
            ע�������contact����������Ϊ�㣬�򲻻��յ�EndContact�ص������ǣ���һ֡�����ܻ��յ�BeginContact�ز���

			not used
		*/
		virtual void PreSolve(b3_Contact* contact, const b3_Manifold* oldManifold)
		{
			VOL_TRACE("Colliding!!!");
			(void)(contact); /* not used */
			(void)(oldManifold); /* not used */
		}

		/*
		    ��ʹ�������������(solver)��ɺ���(inspect)�Ӵ�������ڼ����������á� 
            ע��contact manifold���������������ʱ�䣬����Ӳ���(sub-step)��С����������������(arbitrarily)��
			��ˣ������ڵ��������ݽṹ����ȷ(explicitly)�ṩ�� 
            ע�⣺���������touching��solid��awake��contacts��
		*/
		virtual void PostSolve(b3_Contact* contact, const b3_ContactImpulse* impulse)
		{
			(void)(contact); // not used
			(void)(impulse); // not used
		}
	};

	// AABB��ѯ�Ļص���  Callback class for AABB queries.  See b3_World::Query
	class b3_QueryCallback
	{
	public:
		virtual ~b3_QueryCallback() {}

		// Ϊ��ѯAABB���ҵ���ÿ��fixture����callback��
		// @return false to terminate the query.
		virtual bool ReportFixture(b3_Fixture* fixture) = 0;
	};

	// ray casts�Ļص���  Callback class for ray casts.   See b3_World::RayCast
	class b3_RayCastCallback
	{
	public:
		virtual ~b3_RayCastCallback() {}

		
		// Ϊ��ѯ(query)���ҵ���ÿ��fixture���á�ͨ������һ����������������δ���ray cast:
		// return -1��       ���Դ�fixture������ 
		// return  0��       ��ֹray cast 
		// return  fraction���������������ײ 
		// return  1��       ����û����ײ������ 
		// @param fixture  �����߻��е�fixture 
		// @param point    ��ʼ���� 
		// @param normal   ���㴦�ķ����� 
		// @param fraction ���㴦�����ߵķ��� 
		// @return -1��ʾ���ˣ�0��ʾ��ֹ��fraction��ʾ�����������ײ�㣬1��ʾ����
		virtual float ReportFixture(b3_Fixture* fixture, const glm::vec3& point, const glm::vec3& normal, float fraction) = 0;
	};

}