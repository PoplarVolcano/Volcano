#include "volpch.h"

#include "b3_DynamicTree.h"
#include "b3_Common.h"

namespace Volcano {

	b3_DynamicTree::b3_DynamicTree()
	{
		m_root = b3_NullNode;

		m_nodeCapacity = 16;
		m_nodeCount = 0;
		m_nodes = (b3_TreeNode*)b3_Alloc(m_nodeCapacity * sizeof(b3_TreeNode));
		memset(m_nodes, 0, m_nodeCapacity * sizeof(b3_TreeNode));

		// Build a linked list for the free list.
		for (int i = 0; i < m_nodeCapacity - 1; ++i)
		{
			m_nodes[i].next = i + 1;
			m_nodes[i].height = -1;
		}
		m_nodes[m_nodeCapacity - 1].next = b3_NullNode;
		m_nodes[m_nodeCapacity - 1].height = -1;
		m_freeList = 0;

		m_insertionCount = 0;
	}

	b3_DynamicTree::~b3_DynamicTree()
	{
		b3_Free(m_nodes);
	}

	int b3_DynamicTree::CreateProxy(const b3_AABB& aabb, void* userData)
	{
		int proxyId = AllocateNode();

		/*
		    动态对象的位置、旋转、缩放等变化会影响AABB，需要动态更新。但是根据帧间的相关性，每帧AABB发生的变化并不大。
		    因此在BroadPhase阶段，适当放大AABB有助于减少每次更新量并且不会影响最终结果正确性。
		*/
		glm::vec3 r(b3_AABBExtension, b3_AABBExtension, b3_AABBExtension);
		m_nodes[proxyId].aabb.lowerBound = aabb.lowerBound - r;
		m_nodes[proxyId].aabb.upperBound = aabb.upperBound + r;
		m_nodes[proxyId].userData = userData;
		m_nodes[proxyId].height = 0;
		m_nodes[proxyId].moved = true;

		InsertLeaf(proxyId);

		return proxyId;
	}

	void b3_DynamicTree::DestroyProxy(int proxyId)
	{
		assert(0 <= proxyId && proxyId < m_nodeCapacity);
		assert(m_nodes[proxyId].IsLeaf());

		RemoveLeaf(proxyId);
		FreeNode(proxyId);
	}

	// 从节点池中分配一个节点，必要的话扩张节点池
	int b3_DynamicTree::AllocateNode()
	{
		// 扩张节点池
		if (m_freeList == b3_NullNode)
		{
			assert(m_nodeCount == m_nodeCapacity);

			// The free list is empty. Rebuild a bigger pool.
			b3_TreeNode* oldNodes = m_nodes;
			m_nodeCapacity *= 2;
			m_nodes = (b3_TreeNode*)b3_Alloc(m_nodeCapacity * sizeof(b3_TreeNode));
			memcpy(m_nodes, oldNodes, m_nodeCount * sizeof(b3_TreeNode));
			b3_Free(oldNodes);

			for (int i = m_nodeCount; i < m_nodeCapacity - 1; ++i)
			{
				m_nodes[i].next = i + 1;
				m_nodes[i].height = -1;
			}
			m_nodes[m_nodeCapacity - 1].next = b3_NullNode;
			m_nodes[m_nodeCapacity - 1].height = -1;
			m_freeList = m_nodeCount;
		}

		// 从free list中剥离(Peel)节点。
		int nodeId = m_freeList;
		m_freeList = m_nodes[nodeId].next;
		m_nodes[nodeId].parent = b3_NullNode;
		m_nodes[nodeId].child1 = b3_NullNode;
		m_nodes[nodeId].child2 = b3_NullNode;
		m_nodes[nodeId].height = 0;
		m_nodes[nodeId].userData = nullptr;
		m_nodes[nodeId].moved = false;
		++m_nodeCount;
		return nodeId;
	}

	void b3_DynamicTree::FreeNode(int nodeId)
	{
		assert(0 <= nodeId && nodeId < m_nodeCapacity);
		assert(0 < m_nodeCount);
		m_nodes[nodeId].next = m_freeList;
		m_nodes[nodeId].height = -1;
		m_freeList = nodeId;
		--m_nodeCount;
	}

	void b3_DynamicTree::InsertLeaf(int nodeId)
	{
		++m_insertionCount;

		if (m_root == b3_NullNode)
		{
			m_root = nodeId;
			m_nodes[m_root].parent = b3_NullNode;
			return;
		}

		// 查找此节点的最佳兄弟节点 Find the best sibling for this node
		b3_AABB leafAABB = m_nodes[nodeId].aabb;
		int index = m_root;
		/*
		    从根节点开始遍历树上的节点，对比AABB找到最小代价的节点A.
            对比的规则：对比两个AABB的并集大小(AABB.Combine).
		*/
		while (m_nodes[index].IsLeaf() == false)
		{
			int child1 = m_nodes[index].child1;
			int child2 = m_nodes[index].child2;

			float area = m_nodes[index].aabb.GetPerimeter();  // 取周长作为搜索的权值

			b3_AABB combinedAABB;
			combinedAABB.Combine(m_nodes[index].aabb, leafAABB);
			float combinedArea = combinedAABB.GetPerimeter();

			// 为此节点和新叶子创建新父节点的成本 Cost of creating a new parent for this node and the new leaf
			float cost = 2.0f * combinedArea;

			// 将叶子节点向下推的最低成本 Minimum cost of pushing the leaf further down the tree
			float inheritanceCost = 2.0f * (combinedArea - area);

			// 向下推到child1的成本  Cost of descending into child1
			float cost1;
			if (m_nodes[child1].IsLeaf())
			{
				b3_AABB aabb;
				aabb.Combine(leafAABB, m_nodes[child1].aabb);
				cost1 = aabb.GetPerimeter() + inheritanceCost;
			}
			else
			{
				b3_AABB aabb;
				aabb.Combine(leafAABB, m_nodes[child1].aabb);
				float oldArea = m_nodes[child1].aabb.GetPerimeter();
				float newArea = aabb.GetPerimeter();
				cost1 = (newArea - oldArea) + inheritanceCost;
			}

			// 向下推到child2的成本  Cost of descending into child2
			float cost2;
			if (m_nodes[child2].IsLeaf())
			{
				b3_AABB aabb;
				aabb.Combine(leafAABB, m_nodes[child2].aabb);
				cost2 = aabb.GetPerimeter() + inheritanceCost;
			}
			else
			{
				b3_AABB aabb;
				aabb.Combine(leafAABB, m_nodes[child2].aabb);
				float oldArea = m_nodes[child2].aabb.GetPerimeter();
				float newArea = aabb.GetPerimeter();
				cost2 = newArea - oldArea + inheritanceCost;
			}

			// 根据最低成本下降。 Descend according to the minimum cost.
			if (cost < cost1 && cost < cost2)
			{
				break;
			}

			// 下降 Descend
			if (cost1 < cost2)
			{
				index = child1;
			}
			else
			{
				index = child2;
			}
		}

		int sibling = index;

		/*
            建立一个父节点newParent, sibling的父节点oldParent的原sibling孩子指针指向newParent, sibling和nodeId作为newParent的左右孩子.
            旋转动态树，成为新的平衡树.
		*/
		int oldParent = m_nodes[sibling].parent;
		int newParent = AllocateNode();
		m_nodes[newParent].parent = oldParent;
		m_nodes[newParent].userData = nullptr;
		m_nodes[newParent].aabb.Combine(leafAABB, m_nodes[sibling].aabb);
		m_nodes[newParent].height = m_nodes[sibling].height + 1;

		if (oldParent != b3_NullNode)
		{
			// The sibling was not the root.
			if (m_nodes[oldParent].child1 == sibling)
			{
				m_nodes[oldParent].child1 = newParent;
			}
			else
			{
				m_nodes[oldParent].child2 = newParent;
			}

			m_nodes[newParent].child1 = sibling;
			m_nodes[newParent].child2 = nodeId;
			m_nodes[sibling].parent = newParent;
			m_nodes[nodeId].parent = newParent;
		}
		else
		{
			// The sibling was the root.
			m_nodes[newParent].child1 = sibling;
			m_nodes[newParent].child2 = nodeId;
			m_nodes[sibling].parent = newParent;
			m_nodes[nodeId].parent = newParent;
			m_root = newParent;
		}

		// 往回遍历更新height和AABB  Walk back up the tree fixing heights and AABBs
		index = m_nodes[nodeId].parent;
		while (index != b3_NullNode)
		{
			index = Balance(index);

			int child1 = m_nodes[index].child1;
			int child2 = m_nodes[index].child2;

			assert(child1 != b3_NullNode);
			assert(child2 != b3_NullNode);

			m_nodes[index].height = 1 + glm::max(m_nodes[child1].height, m_nodes[child2].height);
			m_nodes[index].aabb.Combine(m_nodes[child1].aabb, m_nodes[child2].aabb);

			index = m_nodes[index].parent;
		}
		
	}

	/*
	    找到要删除节点的父节点、祖父节点、兄弟节点。
        祖父节点原本指向父节点的孩子指针指向兄弟节点，释放父节点。
        旋转子树，变成平衡二叉树。
	*/
	void b3_DynamicTree::RemoveLeaf(int nodeId)
	{
		if (nodeId== m_root)
		{
			m_root = b3_NullNode;
			return;
		}

		int parent = m_nodes[nodeId].parent;
		int grandParent = m_nodes[parent].parent;
		int sibling;
		if (m_nodes[parent].child1 == nodeId)
		{
			sibling = m_nodes[parent].child2;
		}
		else
		{
			sibling = m_nodes[parent].child1;
		}

		if (grandParent != b3_NullNode)
		{
			// Destroy parent and connect sibling to grandParent.
			if (m_nodes[grandParent].child1 == parent)
			{
				m_nodes[grandParent].child1 = sibling;
			}
			else
			{
				m_nodes[grandParent].child2 = sibling;
			}
			m_nodes[sibling].parent = grandParent;
			FreeNode(parent);

			// Adjust ancestor bounds.
			int index = grandParent;
			while (index != b3_NullNode)
			{
				index = Balance(index);

				int child1 = m_nodes[index].child1;
				int child2 = m_nodes[index].child2;

				m_nodes[index].aabb.Combine(m_nodes[child1].aabb, m_nodes[child2].aabb);
				m_nodes[index].height = 1 + glm::max(m_nodes[child1].height, m_nodes[child2].height);

				index = m_nodes[index].parent;
			}
		}
		else
		{
			m_root = sibling;
			m_nodes[sibling].parent = b3_NullNode;
			FreeNode(parent);
		}

	}

	/*
	    获取当前节点的两个子节点，得到左右子树高度差b.
        若b在[-1,1]区间，说明平衡，不需要旋转。否则上旋子树较高的子节点。
	*/
	int b3_DynamicTree::Balance(int index)
	{
		assert(index != b3_NullNode);

		b3_TreeNode* A = m_nodes + index;
		if (A->IsLeaf() || A->height < 2)
		{
			return index;
		}

		int iB = A->child1;
		int iC = A->child2;
		assert(0 <= iB && iB < m_nodeCapacity);
		assert(0 <= iC && iC < m_nodeCapacity);

		b3_TreeNode* B = m_nodes + iB;
		b3_TreeNode* C = m_nodes + iC;

		int balance = C->height - B->height;

		// Rotate C up
		if (balance > 1)
		{
			int iF = C->child1;
			int iG = C->child2;
			b3_TreeNode* F = m_nodes + iF;
			b3_TreeNode* G = m_nodes + iG;
			assert(0 <= iF && iF < m_nodeCapacity);
			assert(0 <= iG && iG < m_nodeCapacity);

			// Swap A and C
			C->child1 = index;
			C->parent = A->parent;
			A->parent = iC;

			// A's old parent should point to C
			if (C->parent != b3_NullNode)
			{
				if (m_nodes[C->parent].child1 == index)
				{
					m_nodes[C->parent].child1 = iC;
				}
				else
				{
					assert(m_nodes[C->parent].child2 == index);
					m_nodes[C->parent].child2 = iC;
				}
			}
			else
			{
				m_root = iC;
			}

			// Rotate
			if (F->height > G->height)
			{
				C->child2 = iF;
				A->child2 = iG;
				G->parent = index;
				A->aabb.Combine(B->aabb, G->aabb);
				C->aabb.Combine(A->aabb, F->aabb);

				A->height = 1 + glm::max(B->height, G->height);
				C->height = 1 + glm::max(A->height, F->height);
			}
			else
			{
				C->child2 = iG;
				A->child2 = iF;
				F->parent = index;
				A->aabb.Combine(B->aabb, F->aabb);
				C->aabb.Combine(A->aabb, G->aabb);

				A->height = 1 + glm::max(B->height, F->height);
				C->height = 1 + glm::max(A->height, G->height);
			}

			return iC;
		}

		// Rotate B up
		if (balance < -1)
		{
			int iD = B->child1;
			int iE = B->child2;
			b3_TreeNode* D = m_nodes + iD;
			b3_TreeNode* E = m_nodes + iE;
			assert(0 <= iD && iD < m_nodeCapacity);
			assert(0 <= iE && iE < m_nodeCapacity);

			// Swap A and B
			B->child1 = index;
			B->parent = A->parent;
			A->parent = iB;

			// A's old parent should point to B
			if (B->parent != b3_NullNode)
			{
				if (m_nodes[B->parent].child1 == index)
				{
					m_nodes[B->parent].child1 = iB;
				}
				else
				{
					assert(m_nodes[B->parent].child2 == index);
					m_nodes[B->parent].child2 = iB;
				}
			}
			else
			{
				m_root = iB;
			}

			// Rotate
			if (D->height > E->height)
			{
				B->child2 = iD;
				A->child1 = iE;
				E->parent = index;
				A->aabb.Combine(C->aabb, E->aabb);
				B->aabb.Combine(A->aabb, D->aabb);

				A->height = 1 + glm::max(C->height, E->height);
				B->height = 1 + glm::max(A->height, D->height);
			}
			else
			{
				B->child2 = iE;
				A->child1 = iD;
				D->parent = index;
				A->aabb.Combine(C->aabb, D->aabb);
				B->aabb.Combine(A->aabb, E->aabb);

				A->height = 1 + glm::max(C->height, D->height);
				B->height = 1 + glm::max(A->height, E->height);
			}

			return iB;
		}

		return index;
	}

}