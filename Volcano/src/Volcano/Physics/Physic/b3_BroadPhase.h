#pragma once

#include "b3_Collision.h"
#include "b3_Math.h"
#include "b3_DynamicTree.h""
#include "b3_Setting.h"

namespace Volcano {

	// 通常 proxyIdA < proxyIdB
	struct b3_Pair
	{
		int proxyIdA;
		int proxyIdB;
	};

	
	// broad-phase用于计算pairs以及执行体积查询(volume queries)和光线投射。 
    // broad-phase不会维持pairs。相反，它会报告潜在的新pairs。 
    // 客户可以使用新的pairs用于跟踪后续的重叠(overlap)。
	class b3_BroadPhase
	{
	public:
		enum
		{
			e_nullProxy = -1
		};

		b3_BroadPhase();
		~b3_BroadPhase();

	    // 创建代理，加入move缓冲。在调用UpdatePairs之前，不会报告配对.  Pairs are not reported until UpdatePairs is called.
		int CreateProxy(const b3_AABB& aabb, void* userData);
		/// 销毁代理。客户有权删除任何配对。 It is up to the client to remove any pairs.
		void DestroyProxy(int proxyId);

		// 根据需要多次调用MoveProxy，然后调用UpdatePairs以最终确定代理对(proxy pairs)
		void MoveProxy(int proxyId, const b3_AABB& aabb, const glm::vec3& displacement);

		// 调用此函数以在下次调用UpdatePairs时触发对其pairs的重新处理(re-processing)。
		void TouchProxy(int proxyId);

		const b3_AABB& GetFatAABB(int proxyId) const;
		void* GetUserData(int proxyId) const;
		bool TestOverlap(int proxyIdA, int proxyIdB) const;
		int GetProxyCount() const;

		// 为重叠的代理查询AABB。对于与提供的AABB重叠的每个代理，都会调用回调类。
		template <typename T>
		void Query(T* callback, const b3_AABB& aabb) const;

		// 对动态树上的代理进行射线投射Ray-cast。这依赖于回调函数在代理包含shape的情况下执行(perform)精确(exact)的光线投射。 
		// 回调函数还执行一些碰撞过滤。其性能大致等于k*log(n)，其中k是碰撞数，n是动态树中的代理数。
		// @param input 光线投射输入数据。光线从p1延伸到p1+maxFraction*(p2-p1)。
		// @param callback 是一个回调类，为每个被光线击中的代理调用。
		template <typename T>
		void RayCast(T* callback, const b3_RayCastInput& input) const;

		// Shift(变动) the world origin. Useful for large worlds. The body shift formula(公式) is: newOrigin = oldOrigin - newOriginTranslate
		// @param newOriginTranslate 新原点相对于旧原点的变换 
		void ShiftOrigin(const glm::vec3& newOriginTranslate);

		int GetTreeHeight() const;
		int GetTreeBalance() const;
		//float GetTreeQuality() const;

		// 更新pairs。这会导致pairs回调。这个函数只能添加pairs。
		template <typename T>
		void UpdatePairs(T* callback);

	private:

		friend class b3_DynamicTree;

		//添加proxyId到移动缓冲区
		void BufferMove(int proxyId);
		void UnBufferMove(int proxyId);

		// 将pair加入pairBuffer，动态树查询到与代理proxyId的AABB重叠的AABB后调用QueryCallback
        // This is called from b3_DynamicTree::Query when we are gathering pairs.
		bool QueryCallback(int proxyId);
		
		b3_DynamicTree m_tree; // 动态树
		int m_proxyCount;      // 代理数量
		int* m_moveBuffer;     // 移动的缓冲区
		int m_moveCapacity;    // 移动缓冲区的总容量
		int m_moveCount;       // 需要移动的代理数量
		b3_Pair* m_pairBuffer; // pair缓冲区
		int m_pairCapacity;    // pair缓冲区中的总容量
		int m_pairCount;       // pair数量
		int m_queryProxyId;    // 正在检查碰撞的代理id
	};

	inline void b3_BroadPhase::TouchProxy(int proxyId)
	{
		BufferMove(proxyId);
	}

	inline const b3_AABB& b3_BroadPhase::GetFatAABB(int proxyId) const
	{
		return m_tree.GetFatAABB(proxyId);
	}

	inline void* b3_BroadPhase::GetUserData(int proxyId) const
	{
		return m_tree.GetUserData(proxyId);
	}

	inline bool b3_BroadPhase::TestOverlap(int proxyIdA, int proxyIdB) const
	{
		const b3_AABB& aabbA = m_tree.GetFatAABB(proxyIdA);
		const b3_AABB& aabbB = m_tree.GetFatAABB(proxyIdB);
		return b3_TestOverlap(aabbA, aabbB);
	}

	inline int b3_BroadPhase::GetProxyCount() const
	{
		return m_proxyCount;
	}

	template<typename T>
	inline void b3_BroadPhase::Query(T* callback, const b3_AABB& aabb) const
	{
		m_tree.Query(callback, aabb);
	}

	template<typename T>
	inline void b3_BroadPhase::RayCast(T* callback, const b3_RayCastInput& input) const
	{
		m_tree.RayCast(callback, input);
	}

	inline void b3_BroadPhase::ShiftOrigin(const glm::vec3& newOriginTranslate)
	{
		m_tree.ShiftOrigin(newOriginTranslate);
	}

	inline int b3_BroadPhase::GetTreeHeight() const
	{
		return m_tree.GetHeight();
	}

	inline int b3_BroadPhase::GetTreeBalance() const
	{
		return m_tree.GetMaxBalance();
	}

	/*
	inline float b3_BroadPhase::GetTreeQuality() const
	{
		return m_tree.GetAreaRatio();
	}
	*/

	template<typename T>
	inline void b3_BroadPhase::UpdatePairs(T* callback)
	{
		// Reset pair buffer
		m_pairCount = 0;

		// 为所有移动的proxies执行一次动态树的query函数
		for (int i = 0; i < m_moveCount; ++i)
		{
			m_queryProxyId = m_moveBuffer[i];
			if (m_queryProxyId == e_nullProxy)
			{
				continue;
			}

			// 我们必须用fatAABB查询树，这样我们一定会创造出以后可能会碰到pair。
			const b3_AABB& fatAABB = m_tree.GetFatAABB(m_queryProxyId);

			// 查询动态树，将查询结果与m_queryProxyId创建pair并加入pair缓冲
			m_tree.Query(this, fatAABB);
		}

		// 将pair注入callback
		for (int i = 0; i < m_pairCount; ++i)
		{
			b3_Pair* primaryPair = m_pairBuffer + i;
			void* userDataA = m_tree.GetUserData(primaryPair->proxyIdA);
			void* userDataB = m_tree.GetUserData(primaryPair->proxyIdB);

			callback->AddPair(userDataA, userDataB);
		}

		// 清理移动的proxies的move标签
		for (int i = 0; i < m_moveCount; ++i)
		{
			int proxyId = m_moveBuffer[i];
			if (proxyId == e_nullProxy)
			{
				continue;
			}

			m_tree.ClearMoved(proxyId);
		}

		// Reset move buffer
		m_moveCount = 0;
	}

}