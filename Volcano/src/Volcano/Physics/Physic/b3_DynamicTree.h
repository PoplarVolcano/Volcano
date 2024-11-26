#pragma once

#include "b3_Setting.h"
#include "b3_Collision.h"
#include "b3_GrowableStack.h"

#define b3_NullNode (-1)

namespace Volcano {

	/// A node in the dynamic tree. The client does not interact with this directly.
	struct b3_TreeNode
	{
		// 没有子节点的叶节点
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

		// 结点高度 leaf = 0, free node = -1
		int height;

		bool moved;
	};

	/*
	    一个动态的AABB树broad-phase，灵感来自Nathanael Presson的btDbvt。 
        动态树在二叉树中排列(arranges)数据，以加速卷查询(volume queries)和光线投射(ray casts)等查询。叶子是AABB的代理。
	    在树中，我们将代理AABB扩展为b3_fatAABBFactor，使代理AABB大于委托(client)对象。这允许委托对象少量移动，而不会触发树更新。 
        节点是池化(pooled)和可重定位(relocatable)的，因此我们使用节点索引而不是指针。
	*/
	class b3_DynamicTree
	{
	public:
		b3_DynamicTree();
		~b3_DynamicTree();

		// 创建一个新代理，提供一个紧密贴合(tight fitting)的AABB和一个用户数据指针，将新代理插入动态树(新代理的AABB是经过fat的)
		int CreateProxy(const b3_AABB& aabb, void* userData);
		void DestroyProxy(int proxyId);

		// 已应答(swepted)的AABB的代理执行移动。如果代理移动后移出其增肥(fattened)(动态树的父节点)的AABB，则代理将从树中删除并重新插入。否则，函数将立即返回。 
		// @param displacement 位移
		// @return 如果移动后要重新插入代理，则返回true。
		bool MoveProxy(int proxyId, const b3_AABB& aabb, const glm::vec3& displacement);

		/// Get proxy user data.
		/// @return the proxy user data or 0 if the id is invalid.
		void* GetUserData(int proxyId) const;

		bool WasMoved(int proxyId) const;
		void ClearMoved(int proxyId);

		const b3_AABB& GetFatAABB(int proxyId) const;

		// 查询与aabb重叠的代理。对于与aabb重叠的每个代理，都会调用回调类。
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

		/// Validate this tree. For testing.
		//void Validate() const;

		// 在O(N)时间内计算二叉树的高度。不应该经常被调用。
		int GetHeight() const;

		// 获取树中节点的最大平衡。平衡是节点的两个子节点的高度差。
		int GetMaxBalance() const;

		// 获取节点面积之和与根面积之比。
		//float GetAreaRatio() const;
		
		// 获取节点体积之和与根体积之比。
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
		int m_nodeCapacity;   // 结点容量

		int m_freeList;

		int m_insertionCount; // 插入计数
	};

	inline bool b3_DynamicTree::MoveProxy(int proxyId, const b3_AABB& aabb, const glm::vec3& displacement)
	{
		assert(0 <= proxyId && proxyId < m_nodeCapacity);

		assert(m_nodes[proxyId].IsLeaf());

		// 扩增AABB以契合动态树中代理节点的AABB(动态树创建代理时会注入扩增的AABB)
		b3_AABB fatAABB;
		glm::vec3 r(b3_AABBExtension, b3_AABBExtension, b3_AABBExtension);
		fatAABB.lowerBound = aabb.lowerBound - r;
		fatAABB.upperBound = aabb.upperBound + r;

		// 预测AABB移动 Predict AABB movement
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
			// treeAABB包含移动后的object，但它可能过大
			b3_AABB hugeAABB;
			hugeAABB.lowerBound = fatAABB.lowerBound - b3_AABBMultiplier * r;
			hugeAABB.upperBound = fatAABB.upperBound + b3_AABBMultiplier * r;

			if (hugeAABB.Contains(treeAABB))
			{
				// treeAABB包含对象AABB，treeAABB没有过大。不需要更新树。
				return false;
			}

			// 否则treeAABB大了，需要收缩(shrunk)
		}

		RemoveLeaf(proxyId);

		// 移动object
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
		r = glm::normalize(r); // 单位向量p1p2

		float maxFraction = input.maxFraction;

		// 为线段(segment)创建一个aabb
		glm::vec3 pt = p1 + maxFraction * (p2 - p1); // 线段终点的点pt，maxFraction不一定等于1，t点可能在线段p1p2前中后任何位置
		b3_AABB segmentAABB;
		{
			segmentAABB.lowerBound = glm::min(p1, pt);
			segmentAABB.upperBound = glm::max(p1, pt);
		}

		b3_GrowableStack<int, 256> stack;
		stack.Push(m_root);

		// 从根节点开始递归 => 射线与节点相交 => 节点非叶节点 => 栈加入子节点，递归
		//                                    => 节点为叶节点 => 执行callback => 返回值：-1过滤，0终止，fraction光线最近的碰撞点，1继续
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

				// 光线击中物体被阻挡且结束ray-cast
				if (value == 0.0f)
				{
					return;
				}

				// value == input.maxFraction：光线击中物体后未被阻挡，继续ray-cast
				// value == 新fraction：       光线击中物体后被阻挡，在光线起点到碰撞点之间继续ray-cast
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