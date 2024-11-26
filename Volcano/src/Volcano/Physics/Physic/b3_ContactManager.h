#pragma once
#include "b3_BroadPhase.h"

namespace Volcano {

	class b3_Contact;
	class b3_ContactFilter;
	class b3_ContactListener;
	class b3_BlockAllocator;

	// ����3Dworld
	class b3_ContactManager
	{
	public:
		b3_ContactManager();

		// Broad-phase callback. ��b3_BroadPhase::UpdatePairs�е��ã���Ӵּ��pair
		void AddPair(void* proxyUserDataA, void* proxyUserDataB);
		void FindNewContacts();
		void Destroy(b3_Contact* c);
		// ����ʱ�䲽����ߵȼ���ײ���á����ﴦ��contact�б�������ײ�������˺�broad-phaseδ��ײ��contact�ᱻ���١�������narrow phase����
		void Collide();

		b3_BroadPhase m_broadPhase;
		b3_Contact* m_contactList;
		int m_contactCount;
		b3_ContactFilter* m_contactFilter;
		b3_ContactListener* m_contactListener;
		b3_BlockAllocator* m_allocator;
	};

}