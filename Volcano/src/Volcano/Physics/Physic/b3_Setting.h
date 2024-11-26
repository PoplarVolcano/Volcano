#pragma once

namespace Volcano {

// 可调常数 Tunable Constants

#define b3_LengthUnitsPerMeter 1.0f

// 凸多边形上的最大顶点数。您不能将其增加太多，因为b3_BlockAllocator具有最大对象大小。
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

	// 默认分配函数
	void* b3_Alloc_Default(int size);
	void b3_Free_Default(void* mem);

	// 实现此函数以使用您自己的内存分配器。
	// malloc(size);
	inline void* b3_Alloc(int size)
	{
		return b3_Alloc_Default(size);
	}

	// 如果你实现了b3_Alloc，你也应该实现这个函数。
	// free(mem)
	inline void b3_Free(void* mem)
	{
		b3_Free_Default(mem);
	}

}