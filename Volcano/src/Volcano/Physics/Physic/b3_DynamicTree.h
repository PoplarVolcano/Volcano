#pragma once

#include "b3_Setting.h"
#include "b3_Collision.h"
#include "b3_GrowableStack.h"

#define b3_NullNode (-1)

namespace Volcano {

	/// A node in the dynamic tree. The client does not interact with this directly.
	struct b3_TreeNode
	{
		// û���ӽڵ��Ҷ�ڵ�
		bool IsLeaf() const
		{
			return child1 == b3_NullNode;
		}

		/// Enlarged AABB
		b3_AABB aabb;

		void* userData;

		union
		{
			int parent;
			int next;
		};

		int child1;
		int child2;

		// ���߶� leaf = 0, free node = -1
		int height;

		bool moved;
	};

	/*
	    һ����̬��AABB��broad-phase���������Nathanael Presson��btDbvt�� 
        ��̬���ڶ�����������(arranges)���ݣ��Լ��پ��ѯ(volume queries)�͹���Ͷ��(ray casts)�Ȳ�ѯ��Ҷ����AABB�Ĵ���
	    �����У����ǽ�����AABB��չΪb3_fatAABBFactor��ʹ����AABB����ί��(client)����������ί�ж��������ƶ��������ᴥ�������¡� 
        �ڵ��ǳػ�(pooled)�Ϳ��ض�λ(relocatable)�ģ��������ʹ�ýڵ�����������ָ�롣
	*/
	class b3_DynamicTree
	{
	public:
		b3_DynamicTree();
		~b3_DynamicTree();

		// ����һ���´����ṩһ����������(tight fitting)��AABB��һ���û�����ָ�룬���´�����붯̬��(�´����AABB�Ǿ���fat��)
		int CreateProxy(const b3_AABB& aabb, void* userData);
		void DestroyProxy(int proxyId);

		// ��Ӧ��(swepted)��AABB�Ĵ���ִ���ƶ�����������ƶ����Ƴ�������(fattened)(��̬���ĸ��ڵ�)��AABB�������������ɾ�������²��롣���򣬺������������ء� 
		// @param displacement λ��
		// @return ����ƶ���Ҫ���²�������򷵻�true��
		bool MoveProxy(int proxyId, const b3_AABB& aabb, const glm::vec3& displacement);

		/// Get proxy user data.
		/// @return the proxy user data or 0 if the id is invalid.
		void* GetUserData(int proxyId) const;

		bool WasMoved(int proxyId) const;
		void ClearMoved(int proxyId);

		const b3_AABB& GetFatAABB(int proxyId) const;

		// ��ѯ��aabb�ص��Ĵ���������aabb�ص���ÿ������������ûص��ࡣ
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

		/// Validate this tree. For testing.
		//void Validate() const;

		// ��O(N)ʱ���ڼ���������ĸ߶ȡ���Ӧ�þ��������á�
		int GetHeight() const;

		// ��ȡ���нڵ�����ƽ�⡣ƽ���ǽڵ�������ӽڵ�ĸ߶Ȳ
		int GetMaxBalance() const;

		// ��ȡ�ڵ����֮��������֮�ȡ�
		//float GetAreaRatio() const;
		
		// ��ȡ�ڵ����֮��������֮�ȡ�
		//float GetVolumRatio() const;

		/// Build an optimal tree. Very expensive. For testing.
		//void RebuildBottomUp();

		
	private:

		int AllocateNode();
		void FreeNode(int nodeId);

		void InsertLeaf(int nodeId);
		void RemoveLeaf(int nodeId);

		int Balance(int index);
		/*

		int ComputeHeight() const;
		int ComputeHeight(int nodeId) const;

		void ValidateStructure(int index) const;
		void ValidateMetrics(int index) const;
		*/
		int m_root;

		b3_TreeNode* m_nodes;
		int m_nodeCount;
		int m_nodeCapacity;   // �������

		int m_freeList;

		int m_insertionCount; // �������
	};

	inline bool b3_DynamicTree::MoveProxy(int proxyId, const b3_AABB& aabb, const glm::vec3& displacement)
	{
		assert(0 <= proxyId && proxyId < m_nodeCapacity);

		assert(m_nodes[proxyId].IsLeaf());

		// ����AABB�����϶�̬���д���ڵ��AABB(��̬����������ʱ��ע��������AABB)
		b3_AABB fatAABB;
		glm::vec3 r(b3_AABBExtension, b3_AABBExtension, b3_AABBExtension);
		fatAABB.lowerBound = aabb.lowerBound - r;
		fatAABB.upperBound = aabb.upperBound + r;

		// Ԥ��AABB�ƶ� Predict AABB movement
		glm::vec3 d = b3_AABBMultiplier * displacement;

		if (d.x < 0.0f)
		{
			fatAABB.lowerBound.x += d.x;
		}
		else
		{
			fatAABB.upperBound.x += d.x;
		}

		if (d.y < 0.0f)
		{
			fatAABB.lowerBound.y += d.y;
		}
		else
		{
			fatAABB.upperBound.y += d.y;
		}

		if (d.z < 0.0f)
		{
			fatAABB.lowerBound.z += d.z;
		}
		else
		{
			fatAABB.upperBound.z += d.z;
		}

		const b3_AABB& treeAABB = m_nodes[proxyId].aabb;
		//if (treeAABB.Contains(aabb))
		if (treeAABB.Contains(fatAABB))
		{
			// treeAABB�����ƶ����object���������ܹ���
			b3_AABB hugeAABB;
			hugeAABB.lowerBound = fatAABB.lowerBound - b3_AABBMultiplier * r;
			hugeAABB.upperBound = fatAABB.upperBound + b3_AABBMultiplier * r;

			if (hugeAABB.Contains(treeAABB))
			{
				// treeAABB��������AABB��treeAABBû�й��󡣲���Ҫ��������
				return false;
			}

			// ����treeAABB���ˣ���Ҫ����(shrunk)
		}

		RemoveLeaf(proxyId);

		// �ƶ�object
		m_nodes[proxyId].aabb = fatAABB;

		InsertLeaf(proxyId);

		m_nodes[proxyId].moved = true;

		return true;
	}

	inline void* b3_DynamicTree::GetUserData(int proxyId) const
	{
		assert(0 <= proxyId && proxyId < m_nodeCapacity);
		return m_nodes[proxyId].userData;
	}

	inline bool b3_DynamicTree::WasMoved(int proxyId) const
	{
		assert(0 <= proxyId && proxyId < m_nodeCapacity);
		return m_nodes[proxyId].moved;
	}

	inline void b3_DynamicTree::ClearMoved(int proxyId)
	{
		assert(0 <= proxyId && proxyId < m_nodeCapacity);
		m_nodes[proxyId].moved = false;
	}

	inline const b3_AABB& b3_DynamicTree::GetFatAABB(int proxyId) const
	{
		assert(0 <= proxyId && proxyId < m_nodeCapacity);
		return m_nodes[proxyId].aabb;
	}

	template<typename T>
	inline void b3_DynamicTree::Query(T* callback, const b3_AABB& aabb) const
	{
		b3_GrowableStack<int, 256> stack;
		stack.Push(m_root);

		while (stack.GetCount() > 0)
		{
			int nodeId = stack.Pop();
			if (nodeId == b3_NullNode)
			{
				continue;
			}

			const b3_TreeNode* node = m_nodes + nodeId;

			if (b3_TestOverlap(node->aabb, aabb))
			{
				if (node->IsLeaf())
				{
					bool proceed = callback->QueryCallback(nodeId);
					if (proceed == false)
					{
						return;
					}
				}
				else
				{
					stack.Push(node->child1);
					stack.Push(node->child2);
				}
			}
		}
	}

	template<typename T>
	inline void b3_DynamicTree::RayCast(T* callback, const b3_RayCastInput& input) const
	{
		glm::vec3 p1 = input.p1;
		glm::vec3 p2 = input.p2;
		glm::vec3 r = p2 - p1;
		assert(glm::dot(r, r) > 0.0f);
		r = glm::normalize(r); // ��λ����p1p2

		float maxFraction = input.maxFraction;

		// Ϊ�߶�(segment)����һ��aabb
		glm::vec3 pt = p1 + maxFraction * (p2 - p1); // �߶��յ�ĵ�pt��maxFraction��һ������1��t��������߶�p1p2ǰ�к��κ�λ��
		b3_AABB segmentAABB;
		{
			segmentAABB.lowerBound = glm::min(p1, pt);
			segmentAABB.upperBound = glm::max(p1, pt);
		}

		b3_GrowableStack<int, 256> stack;
		stack.Push(m_root);

		// �Ӹ��ڵ㿪ʼ�ݹ� => ������ڵ��ཻ => �ڵ��Ҷ�ڵ� => ջ�����ӽڵ㣬�ݹ�
		//                                    => �ڵ�ΪҶ�ڵ� => ִ��callback => ����ֵ��-1���ˣ�0��ֹ��fraction�����������ײ�㣬1����
		while (stack.GetCount() > 0)
		{
			int nodeId = stack.Pop();
			if (nodeId == b3_NullNode)
			{
				continue;
			}

			const b3_TreeNode* node = m_nodes + nodeId;

			if (b3_TestOverlap(node->aabb, segmentAABB) == false)
			{
				continue;
			}

			if (b3_RayAABBIntersect(p1, pt, node->aabb) < 0.0f)
				continue;

			if (node->IsLeaf())
			{
				b3_RayCastInput subInput;
				subInput.p1 = p1;
				subInput.p2 = p2;
				subInput.maxFraction = maxFraction;

				float value = callback->RayCastCallback(subInput, nodeId);

				// ���߻������屻�赲�ҽ���ray-cast
				if (value == 0.0f)
				{
					return;
				}

				// value == input.maxFraction�����߻��������δ���赲������ray-cast
				// value == ��fraction��       ���߻���������赲���ڹ�����㵽��ײ��֮�����ray-cast
				if (value > 0.0f)
				{
					// Update segment bounding box.
					maxFraction = value;
					glm::vec3 pt = p1 + maxFraction * (p2 - p1);
					segmentAABB.lowerBound = glm::min(p1, pt);
					segmentAABB.upperBound = glm::max(p1, pt);
				}
			}
			else
			{
				stack.Push(node->child1);
				stack.Push(node->child2);
			}
		}
	}

	inline void b3_DynamicTree::ShiftOrigin(const glm::vec3& newOriginTranslate)
	{
		for (int i = 0; i < m_nodeCapacity; ++i)
		{
			m_nodes[i].aabb.lowerBound -= newOriginTranslate;
			m_nodes[i].aabb.upperBound -= newOriginTranslate;
		}
	}

	inline int b3_DynamicTree::GetHeight() const
	{
		if (m_root == b3_NullNode)
		{
			return 0;
		}
		return m_nodes[m_root].height;
	}

	inline int b3_DynamicTree::GetMaxBalance() const
	{
		int maxBalance = 0;
		for (int i = 0; i < m_nodeCapacity; ++i)
		{
			const b3_TreeNode* node = m_nodes + i;
			if (node->height <= 1)
			{
				continue;
			}

			assert(node->IsLeaf() == false);

			int child1 = node->child1;
			int child2 = node->child2;
			int balance = glm::abs(m_nodes[child2].height - m_nodes[child1].height);
			maxBalance = glm::max(maxBalance, balance);
		}

		return maxBalance;
	}

}