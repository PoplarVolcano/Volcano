#pragma once

#include "b3_Collision.h"
#include "b3_Math.h"
#include "b3_DynamicTree.h""
#include "b3_Setting.h"

namespace Volcano {

	// ͨ�� proxyIdA < proxyIdB
	struct b3_Pair
	{
		int proxyIdA;
		int proxyIdB;
	};

	
	// broad-phase���ڼ���pairs�Լ�ִ�������ѯ(volume queries)�͹���Ͷ�䡣 
    // broad-phase����ά��pairs���෴�����ᱨ��Ǳ�ڵ���pairs�� 
    // �ͻ�����ʹ���µ�pairs���ڸ��ٺ������ص�(overlap)��
	class b3_BroadPhase
	{
	public:
		enum
		{
			e_nullProxy = -1
		};

		b3_BroadPhase();
		~b3_BroadPhase();

	    // ������������move���塣�ڵ���UpdatePairs֮ǰ�����ᱨ�����.  Pairs are not reported until UpdatePairs is called.
		int CreateProxy(const b3_AABB& aabb, void* userData);
		/// ���ٴ����ͻ���Ȩɾ���κ���ԡ� It is up to the client to remove any pairs.
		void DestroyProxy(int proxyId);

		// ������Ҫ��ε���MoveProxy��Ȼ�����UpdatePairs������ȷ�������(proxy pairs)
		void MoveProxy(int proxyId, const b3_AABB& aabb, const glm::vec3& displacement);

		// ���ô˺��������´ε���UpdatePairsʱ��������pairs�����´���(re-processing)��
		void TouchProxy(int proxyId);

		const b3_AABB& GetFatAABB(int proxyId) const;
		void* GetUserData(int proxyId) const;
		bool TestOverlap(int proxyIdA, int proxyIdB) const;
		int GetProxyCount() const;

		// Ϊ�ص��Ĵ����ѯAABB���������ṩ��AABB�ص���ÿ������������ûص��ࡣ
		template <typename T>
		void Query(T* callback, const b3_AABB& aabb) const;

		// �Զ�̬���ϵĴ����������Ͷ��Ray-cast���������ڻص������ڴ������shape�������ִ��(perform)��ȷ(exact)�Ĺ���Ͷ�䡣 
		// �ص�������ִ��һЩ��ײ���ˡ������ܴ��µ���k*log(n)������k����ײ����n�Ƕ�̬���еĴ�������
		// @param input ����Ͷ���������ݡ����ߴ�p1���쵽p1+maxFraction*(p2-p1)��
		// @param callback ��һ���ص��࣬Ϊÿ�������߻��еĴ�����á�
		template <typename T>
		void RayCast(T* callback, const b3_RayCastInput& input) const;

		// Shift(�䶯) the world origin. Useful for large worlds. The body shift formula(��ʽ) is: newOrigin = oldOrigin - newOriginTranslate
		// @param newOriginTranslate ��ԭ������ھ�ԭ��ı任 
		void ShiftOrigin(const glm::vec3& newOriginTranslate);

		int GetTreeHeight() const;
		int GetTreeBalance() const;
		//float GetTreeQuality() const;

		// ����pairs����ᵼ��pairs�ص����������ֻ�����pairs��
		template <typename T>
		void UpdatePairs(T* callback);

	private:

		friend class b3_DynamicTree;

		//���proxyId���ƶ�������
		void BufferMove(int proxyId);
		void UnBufferMove(int proxyId);

		// ��pair����pairBuffer����̬����ѯ�������proxyId��AABB�ص���AABB�����QueryCallback
        // This is called from b3_DynamicTree::Query when we are gathering pairs.
		bool QueryCallback(int proxyId);
		
		b3_DynamicTree m_tree; // ��̬��
		int m_proxyCount;      // ��������
		int* m_moveBuffer;     // �ƶ��Ļ�����
		int m_moveCapacity;    // �ƶ���������������
		int m_moveCount;       // ��Ҫ�ƶ��Ĵ�������
		b3_Pair* m_pairBuffer; // pair������
		int m_pairCapacity;    // pair�������е�������
		int m_pairCount;       // pair����
		int m_queryProxyId;    // ���ڼ����ײ�Ĵ���id
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

		// Ϊ�����ƶ���proxiesִ��һ�ζ�̬����query����
		for (int i = 0; i < m_moveCount; ++i)
		{
			m_queryProxyId = m_moveBuffer[i];
			if (m_queryProxyId == e_nullProxy)
			{
				continue;
			}

			// ���Ǳ�����fatAABB��ѯ������������һ���ᴴ����Ժ���ܻ�����pair��
			const b3_AABB& fatAABB = m_tree.GetFatAABB(m_queryProxyId);

			// ��ѯ��̬��������ѯ�����m_queryProxyId����pair������pair����
			m_tree.Query(this, fatAABB);
		}

		// ��pairע��callback
		for (int i = 0; i < m_pairCount; ++i)
		{
			b3_Pair* primaryPair = m_pairBuffer + i;
			void* userDataA = m_tree.GetUserData(primaryPair->proxyIdA);
			void* userDataB = m_tree.GetUserData(primaryPair->proxyIdB);

			callback->AddPair(userDataA, userDataB);
		}

		// �����ƶ���proxies��move��ǩ
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