#pragma once
#include "b3_BroadPhase.h"

namespace Volcano {

	class b3_Contact;
	class b3_ContactFilter;
	class b3_ContactListener;
	class b3_BlockAllocator;

	// 代表3Dworld
	class b3_ContactManager
	{
	public:
		b3_ContactManager();

		// Broad-phase callback. 在b3_BroadPhase::UpdatePairs中调用，添加粗检测pair
		void AddPair(void* proxyUserDataA, void* proxyUserDataB);
		void FindNewContacts();
		void Destroy(b3_Contact* c);
		// 这是时间步的最高等级碰撞调用。这里处理contact列表所有碰撞。被过滤和broad-phase未碰撞的contact会被销毁。最后进行narrow phase处理
		void Collide();

		b3_BroadPhase m_broadPhase;
		b3_Contact* m_contactList;
		int m_contactCount;
		b3_ContactFilter* m_contactFilter;
		b3_ContactListener* m_contactListener;
		b3_BlockAllocator* m_allocator;
	};

}