#include "volpch.h"

#include "b3_BroadPhase.h"

namespace Volcano {
	b3_BroadPhase::b3_BroadPhase()
	{
		m_proxyCount = 0;

		m_pairCapacity = 16;
		m_pairCount = 0;
		m_pairBuffer = (b3_Pair*)b3_Alloc(m_pairCapacity * sizeof(b3_Pair));

		m_moveCapacity = 16;
		m_moveCount = 0;
		m_moveBuffer = (int*)b3_Alloc(m_moveCapacity * sizeof(int));
	}

	b3_BroadPhase::~b3_BroadPhase()
	{
		b3_Free(m_moveBuffer);
		b3_Free(m_pairBuffer);
	}

	int b3_BroadPhase::CreateProxy(const b3_AABB& aabb, void* userData)
	{
		int proxyId = m_tree.CreateProxy(aabb, userData);
		++m_proxyCount;
		BufferMove(proxyId);
		return proxyId;
	}

	void b3_BroadPhase::DestroyProxy(int proxyId)
	{
		UnBufferMove(proxyId);
		--m_proxyCount;
		m_tree.DestroyProxy(proxyId);
	}

	void b3_BroadPhase::MoveProxy(int proxyId, const b3_AABB& aabb, const glm::vec3& displacement)
	{
		bool buffer = m_tree.MoveProxy(proxyId, aabb, displacement);
		if (buffer)
		{
			BufferMove(proxyId);
		}
	}

	void b3_BroadPhase::BufferMove(int proxyId)
	{
		// 扩容
		if (m_moveCount == m_moveCapacity)
		{
			int* oldBuffer = m_moveBuffer;
			m_moveCapacity *= 2;
			m_moveBuffer = (int*)b3_Alloc(m_moveCapacity * sizeof(int));
			memcpy(m_moveBuffer, oldBuffer, m_moveCount * sizeof(int));
			b3_Free(oldBuffer);
		}

		m_moveBuffer[m_moveCount] = proxyId;
		++m_moveCount;
	}

	void b3_BroadPhase::UnBufferMove(int proxyId)
	{
		for (int i = 0; i < m_moveCount; ++i)
		{
			if (m_moveBuffer[i] == proxyId)
			{
				m_moveBuffer[i] = e_nullProxy;
			}
		}
	}

	bool b3_BroadPhase::QueryCallback(int proxyId)
	{
		// A proxy cannot form a pair with itself.
		if (proxyId == m_queryProxyId)
		{
			return true;
		}

		const bool moved = m_tree.WasMoved(proxyId);
		if (moved && proxyId > m_queryProxyId)
		{
			//这两个代理都在移动。避免重复(duplicate)配对。(m_queryProxyId的值在UpdatePairs函数中遍历moveBuffer时获取)
			return true;
		}

		// Grow the pair buffer as needed.
		if (m_pairCount == m_pairCapacity)
		{
			b3_Pair* oldBuffer = m_pairBuffer;
			m_pairCapacity = m_pairCapacity + (m_pairCapacity >> 1);
			m_pairBuffer = (b3_Pair*)b3_Alloc(m_pairCapacity * sizeof(b3_Pair));
			memcpy(m_pairBuffer, oldBuffer, m_pairCount * sizeof(b3_Pair));
			b3_Free(oldBuffer);
		}

		m_pairBuffer[m_pairCount].proxyIdA = glm::min(proxyId, m_queryProxyId);
		m_pairBuffer[m_pairCount].proxyIdB = glm::max(proxyId, m_queryProxyId);
		++m_pairCount;

		return true;
	}

}
