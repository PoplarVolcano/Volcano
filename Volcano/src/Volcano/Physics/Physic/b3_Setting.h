#pragma once

namespace Volcano {

// �ɵ����� Tunable Constants

#define b3_LengthUnitsPerMeter 1.0f

// ͹������ϵ���󶥵����������ܽ�������̫�࣬��Ϊb3_BlockAllocator�����������С��
#define b3_MaxBoxVertices	8

	// User data

	/// You can define this to inject whatever data you want in b3_Body
	struct b3_BodyUserData
	{
		b3_BodyUserData()
		{
			pointer = 0;
		}

		/// For legacy compatibility
		uint64_t pointer;
	};

	/// You can define this to inject whatever data you want in b3_Fixture
	struct b3_FixtureUserData
	{
		b3_FixtureUserData()
		{
			pointer = 0;
		}

		/// For legacy compatibility
		uint64_t pointer;
	};

	/// You can define this to inject whatever data you want in b3_Joint
	struct b3_JointUserData
	{
		b3_JointUserData()
		{
			pointer = 0;
		}

		/// For legacy compatibility
		uint64_t pointer;
	};


	// Memory Allocation

	// Ĭ�Ϸ��亯��
	void* b3_Alloc_Default(int size);
	void b3_Free_Default(void* mem);

	// ʵ�ִ˺�����ʹ�����Լ����ڴ��������
	// malloc(size);
	inline void* b3_Alloc(int size)
	{
		return b3_Alloc_Default(size);
	}

	// �����ʵ����b3_Alloc����ҲӦ��ʵ�����������
	// free(mem)
	inline void b3_Free(void* mem)
	{
		b3_Free_Default(mem);
	}

}