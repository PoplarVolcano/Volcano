#pragma once

#include "b3_BlockAllocator.h"
#include "b3_StackAllocator.h"
#include "b3_Math.h"
#include "b3_ContactManager.h"
#include "b3_TimeStep.h"
#include "b3_WorldCallbacks.h"

namespace Volcano {

	struct b3_BodyDef;
	struct b3_JointDef;
	struct b3_Color;
	class b3_Body;
	class b3_Joint;
	class b3_Draw;

	// �����������������ʵ�塢��̬ģ����첽��ѯ(asynchronous queries)��������Ҳ�и�Ч���ڴ����������
	class b3_World 
	{
	public:
		b3_World(const glm::vec3& gravity);
		// �������磬������������ʵ�壬�ͷ����ж�(heap)�ڴ�
		~b3_World();

		// ע��һ�����ټ�������
		void SetDestructionListener(b3_DestructionListener* listener); 

		//ע��contact���������ṩ����ײ���ض����ơ�
		void SetContactFilter(b3_ContactFilter* filter); 

		// ע��contact�¼���������
		void SetContactListener(b3_ContactListener* listener); 

		// Ϊ����ͼ��(debug drawing)ע��һ������(routine)�����Ի�ͼ�������ڲ�ͨ��b3_World:��DebugDraw�������á�
		//void SetDebugDraw(b3_Draw* debugDraw);

		// ���ݶ���(definition)�������塣δ����(retained)�Ըö�������á� 
		// @warning �˺����ڻص��ڼ䱻������
		b3_Body* CreateBody(const b3_BodyDef* def); 

		// ���ݶ���(definition)���ٸ��塣δ�����Ըö�������á��˺����ڻص��ڼ䱻������ 
		// @warning �⽫�Զ�ɾ�����й�������״�͹ؽڡ� 
		void DestroyBody(b3_Body* body);

		// �����ؽ��Խ�ʵ��Լ��(constrain)��һ��δ�����Ըö�������á�����ܻᵼ�����ӵ�bodyֹͣ��ײ(cease colliding)��
		// ע�������ؽڲ��ỽ��body
		// @warning �˺����ڻص��ڼ䱻������
		b3_Joint* CreateJoint(const b3_JointDef* def);

		// ����һ���ؽڡ�����ܻᵼ�����ӵ�body��ʼ��ײ��
		// @warning���˺����ڻص��ڼ䱻������
		void DestroyJoint(b3_Joint* joint); 

		
		// ִ��һ��ʱ�䲽���⽫ִ����ײ���(detection)���ϲ�(integration)��Լ�����(constraint solution)�� 
		// @param timeStep ����ģ���ʱ��������Ӧ�仯(vary)�� 
		// @param velocityIterations �ٶ�Լ�������(velocity constraint solver)��velocityIterations�� 
		// @param positionIterations λ��Լ�������(position constraint solver)��positionIterations��
		void Step(float timeStep, int velocityIterations, int positionIterations);

		/*
			�ֶ�(Manually)�������body�ϵ�������(force buffer)��Ĭ������£�ÿ�ε���Step�󶼻��Զ������(forces)��ͨ������SetAutoClearForces���޸�Ĭ����Ϊ�� 
			�˹��ܵ�Ŀ����֧���Ӳ���(sub-stepping)���Ӳ���ͨ�������ڿɱ�֡���±��̶ֹ���С��ʱ�䲽���� 
			����ִ���Ӳ���ʱ���㽫���������Զ��������������һ����Ϸѭ����������Ӳ�������ClearForces�� 
		*/
		void ClearForces();

		/*
			��ѯ�������п���(potentially)���ṩ��AABB�ص���fixtures�� 
			@param callback �ص��û�ʵ�ֵĻص��ࡣ callback a user implemented callback class.
			@param aabb aabb��ѯ��aabb the query box.
		*/
		void QueryAABB(b3_QueryCallback* callback, const b3_AABB& aabb) const;

		/*
			����Ͷ��(Ray-cast)������·���ϵ�����fixtures����Ļص��������ǻ������㡢����㻹��n��(closest point, any point, or n-points)�� 
			����Ͷ�����԰�������shape�� 
			@param callback �ص��û�ʵ�ֵĻص��ࡣ 
			@param point1 ������� 
			@param point2 �����յ�
		*/
		void RayCast(b3_RayCastCallback* callback, const glm::vec3& point1, const glm::vec3& point2) const;

		// ��ȡworld body list�����ڷ��ص�body��ʹ��b3_Body::GetNext��ȡworld list�е���һ��body��nullptr��ʾ�б�Ľ�����@return head of the world body list��
		b3_Body* GetBodyList(); 
		const b3_Body* GetBodyList() const;

		// ��ȡworld joint list�����ڷ��ص�joint��ʹ��b3_Joint::GetNext��ȡworld list����һ��joint��nullptr��ʾ�б�Ľ����� @return the head of the world joint list.
		b3_Joint* GetJointList();
		const b3_Joint* GetJointList() const;

		/*
			��ȡworld contact list�����ڷ��ص�contact��ʹ��b3_Contact::GetNext��ȡworld list�е���һ��contact��nullptr��ʾ�б������ 
			@return the head of the world contact list.
			@warning contacts����һ��ʱ�䲽�����м䴴�������ٵġ�
			ʹ��b3_ContactListener���ⶪʧcontacts��
		*/
		b3_Contact* GetContactList();
		const b3_Contact* GetContactList() const;
		void SetAllowSleeping(bool flag);
		bool GetAllowSleeping() const { return m_allowSleep; }
		void SetWarmStarting(bool flag) { m_warmStarting = flag; }
		bool GetWarmStarting() const { return m_warmStarting; }
		void SetContinuousPhysics(bool flag) { m_continuousPhysics = flag; }
		bool GetContinuousPhysics() const { return m_continuousPhysics; }
		void SetSubStepping(bool flag) { m_subStepping = flag; }
		bool GetSubStepping() const { return m_subStepping; }
		int GetProxyCount() const;
		int GetTreeHeight() const;
		int GetTreeBalance() const;
		// ��ȡ��̬������������(quality metric) The smaller the better. The minimum is 1.
		//float GetTreeQuality() const;
		int GetBodyCount() const;
		int GetJointCount() const; 
		int GetContactCount() const ;
		void SetGravity(const glm::vec3& gravity);
		glm::vec3 GetGravity() const;
		bool IsLocked() const;
		// �����Ƿ�ÿ��ʱ�䲽���Զ������(force)��
		void SetAutoClearForces(bool flag);
		// �Ƿ�ÿ��ʱ�䲽���Զ������(force)��
		bool GetAutoClearForces() const;
		// Shift(�䶯) the world origin. Useful for large worlds. The body shift formula(��ʽ) is: newOrigin = oldOrigin - newOriginTranslate
		// @param newOriginTranslate ��ԭ������ھ�ԭ��ı任 
		void ShiftOrigin(const glm::vec3& newOriginTranslate);
		const b3_ContactManager& GetContactManager() const;
		const b3_Profile& GetProfile() const;
		// Dump the world into the log file. @warning this should be called outside of a time step.
		//void Dump();

		// ���ô�����ɻ���shape���������Ի�ͼ(debug draw)���ݡ����ǹ���(intentionally)��const�ġ�
		//void DebugDraw();

	private:

		friend class b3_Body;
		friend class b3_Fixture;

		b3_World(const b3_World&) = delete;
		void operator=(const b3_World&) = delete;

		// Ѱ��islands���ϲ�������Լ��(constraints)������λ��Լ��
		void Solve(const b3_TimeStep& step);
		void SolveTOI(const b3_TimeStep& step);

		//void DrawShape(b3_Fixture* shape, const b3_Transform& transform, const b3_Color& color);

		b3_BlockAllocator m_blockAllocator; // �������
		b3_StackAllocator m_stackAllocator; // ջ������

		b3_ContactManager m_contactManager;

		b3_Body* m_bodyList;
		b3_Joint* m_jointList;
		int m_bodyCount;
		int m_jointCount;
		glm::vec3 m_gravity;
		bool m_allowSleep;

		b3_DestructionListener* m_destructionListener;
		//b3_Draw* m_debugDraw;

		// ��һ֡dt�ĵ����������ڼ���ʱ�䲽����(time step ratio)����֧�ֿɱ�ʱ�䲽��
		float m_inv_deltaTime0;


		bool m_newContacts;  // �ؽ�Contacts��־
		bool m_locked;	     // ������־
		bool m_clearForces;  // ����Ӧ����־
		bool m_stepComplete; // ʱ�䲽��ɱ��

	    // These are for debugging the solver.
		bool m_warmStarting;
		bool m_continuousPhysics;
		bool m_subStepping;

		b3_Profile m_profile;

	};

	inline b3_Body* b3_World::GetBodyList()
	{
		return m_bodyList;
	}

	inline const b3_Body* b3_World::GetBodyList() const
	{
		return m_bodyList;
	}

	inline b3_Joint* b3_World::GetJointList()
	{
		return m_jointList;
	}

	inline const b3_Joint* b3_World::GetJointList() const
	{
		return m_jointList;
	}

	inline b3_Contact* b3_World::GetContactList()
	{
		return m_contactManager.m_contactList;
	}

	inline const b3_Contact* b3_World::GetContactList() const
	{
		return m_contactManager.m_contactList;
	}

	inline int b3_World::GetBodyCount() const
	{
		return m_bodyCount;
	}

	inline int b3_World::GetJointCount() const
	{
		return m_jointCount;
	}

	inline int b3_World::GetContactCount() const
	{
		return m_contactManager.m_contactCount;
	}

	inline void b3_World::SetGravity(const glm::vec3& gravity)
	{
		m_gravity = gravity;
	}

	inline glm::vec3 b3_World::GetGravity() const
	{
		return m_gravity;
	}

	inline bool b3_World::IsLocked() const
	{
		return m_locked;
	}

	inline void b3_World::SetAutoClearForces(bool flag)
	{
		m_clearForces = flag;
	}

	/// Get the flag that controls automatic clearing of forces after each time step.
	inline bool b3_World::GetAutoClearForces() const
	{
		return m_clearForces;
	}

	inline const b3_ContactManager& b3_World::GetContactManager() const
	{
		return m_contactManager;
	}

	inline const b3_Profile& b3_World::GetProfile() const
	{
		return m_profile;
	}

}