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

	// 世界类管理所有物理实体、动态模拟和异步查询(asynchronous queries)。世界类也有高效的内存管理能力。
	class b3_World 
	{
	public:
		b3_World(const glm::vec3& gravity);
		// 销毁世界，销毁所有物理实体，释放所有堆(heap)内存
		~b3_World();

		// 注册一个销毁监听器。
		void SetDestructionListener(b3_DestructionListener* listener); 

		//注册contact过滤器以提供对碰撞的特定控制。
		void SetContactFilter(b3_ContactFilter* filter); 

		// 注册contact事件侦听器。
		void SetContactListener(b3_ContactListener* listener); 

		// 为调试图形(debug drawing)注册一个例程(routine)。调试绘图函数在内部通过b3_World:：DebugDraw方法调用。
		//void SetDebugDraw(b3_Draw* debugDraw);

		// 根据定义(definition)创建刚体。未保留(retained)对该定义的引用。 
		// @warning 此函数在回调期间被锁定。
		b3_Body* CreateBody(const b3_BodyDef* def); 

		// 根据定义(definition)销毁刚体。未保留对该定义的引用。此函数在回调期间被锁定。 
		// @warning 这将自动删除所有关联的形状和关节。 
		void DestroyBody(b3_Body* body);

		// 创建关节以将实体约束(constrain)在一起。未保留对该定义的引用。这可能会导致连接的body停止碰撞(cease colliding)。
		// 注：创建关节不会唤醒body
		// @warning 此函数在回调期间被锁定。
		b3_Joint* CreateJoint(const b3_JointDef* def);

		// 销毁一个关节。这可能会导致连接的body开始碰撞。
		// @warning：此函数在回调期间被锁定。
		void DestroyJoint(b3_Joint* joint); 

		
		// 执行一个时间步。这将执行碰撞检测(detection)、合并(integration)和约束求解(constraint solution)。 
		// @param timeStep 用于模拟的时间量，不应变化(vary)。 
		// @param velocityIterations 速度约束求解器(velocity constraint solver)的velocityIterations。 
		// @param positionIterations 位置约束求解器(position constraint solver)的positionIterations。
		void Step(float timeStep, int velocityIterations, int positionIterations);

		/*
			手动(Manually)清除所有body上的力缓冲(force buffer)。默认情况下，每次调用Step后都会自动清除力(forces)。通过调用SetAutoClearForces来修改默认行为。 
			此功能的目的是支持子步进(sub-stepping)。子步进通常用于在可变帧率下保持固定大小的时间步长。 
			当你执行子步骤时，你将禁用力的自动清除，而不是在一次游戏循环完成所有子步骤后调用ClearForces。 
		*/
		void ClearForces();

		/*
			查询世界所有可能(potentially)与提供的AABB重叠的fixtures。 
			@param callback 回调用户实现的回调类。 callback a user implemented callback class.
			@param aabb aabb查询框。aabb the query box.
		*/
		void QueryAABB(b3_QueryCallback* callback, const b3_AABB& aabb) const;

		/*
			光线投射(Ray-cast)出光线路径上的所有fixtures。你的回调控制你是获得最近点、任意点还是n点(closest point, any point, or n-points)。 
			光线投射会忽略包含起点的shape。 
			@param callback 回调用户实现的回调类。 
			@param point1 射线起点 
			@param point2 射线终点
		*/
		void RayCast(b3_RayCastCallback* callback, const glm::vec3& point1, const glm::vec3& point2) const;

		// 获取world body list。对于返回的body，使用b3_Body::GetNext获取world list中的下一个body。nullptr表示列表的结束。@return head of the world body list。
		b3_Body* GetBodyList(); 
		const b3_Body* GetBodyList() const;

		// 获取world joint list。对于返回的joint，使用b3_Joint::GetNext获取world list的下一个joint。nullptr表示列表的结束。 @return the head of the world joint list.
		b3_Joint* GetJointList();
		const b3_Joint* GetJointList() const;

		/*
			获取world contact list。对于返回的contact，使用b3_Contact::GetNext获取world list中的下一个contact。nullptr表示列表结束。 
			@return the head of the world contact list.
			@warning contacts是在一个时间步长的中间创建和销毁的。
			使用b3_ContactListener避免丢失contacts。
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
		// 获取动态树的质量度量(quality metric) The smaller the better. The minimum is 1.
		//float GetTreeQuality() const;
		int GetBodyCount() const;
		int GetJointCount() const; 
		int GetContactCount() const ;
		void SetGravity(const glm::vec3& gravity);
		glm::vec3 GetGravity() const;
		bool IsLocked() const;
		// 设置是否每个时间步后自动清除力(force)。
		void SetAutoClearForces(bool flag);
		// 是否每个时间步后自动清除力(force)。
		bool GetAutoClearForces() const;
		// Shift(变动) the world origin. Useful for large worlds. The body shift formula(公式) is: newOrigin = oldOrigin - newOriginTranslate
		// @param newOriginTranslate 新原点相对于旧原点的变换 
		void ShiftOrigin(const glm::vec3& newOriginTranslate);
		const b3_ContactManager& GetContactManager() const;
		const b3_Profile& GetProfile() const;
		// Dump the world into the log file. @warning this should be called outside of a time step.
		//void Dump();

		// 调用此命令可绘制shape和其他调试绘图(debug draw)数据。这是故意(intentionally)非const的。
		//void DebugDraw();

	private:

		friend class b3_Body;
		friend class b3_Fixture;

		b3_World(const b3_World&) = delete;
		void operator=(const b3_World&) = delete;

		// 寻找islands，合并并结算约束(constraints)，结算位置约束
		void Solve(const b3_TimeStep& step);
		void SolveTOI(const b3_TimeStep& step);

		//void DrawShape(b3_Fixture* shape, const b3_Transform& transform, const b3_Color& color);

		b3_BlockAllocator m_blockAllocator; // 块分配器
		b3_StackAllocator m_stackAllocator; // 栈分配器

		b3_ContactManager m_contactManager;

		b3_Body* m_bodyList;
		b3_Joint* m_jointList;
		int m_bodyCount;
		int m_jointCount;
		glm::vec3 m_gravity;
		bool m_allowSleep;

		b3_DestructionListener* m_destructionListener;
		//b3_Draw* m_debugDraw;

		// 上一帧dt的倒数，这用于计算时间步长比(time step ratio)，以支持可变时间步长
		float m_inv_deltaTime0;


		bool m_newContacts;  // 重建Contacts标志
		bool m_locked;	     // 锁定标志
		bool m_clearForces;  // 清理应力标志
		bool m_stepComplete; // 时间步完成标记

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