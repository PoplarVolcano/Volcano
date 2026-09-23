#pragma once

#include <mutex>
#include <cassert>
#include <stdint.h>

namespace volstl
{
	/*
	分级内存池（Segregated Storage Memory Pool），专门为频繁分配/释放大量小对象的场景设计
	提前向操作系统申请大块内存（16KB），手动切成固定大小的“小格子”，用链表串起来。
	当用户申请内存时，直接给你一个“格子”；释放时，把“格子”放回链表。这彻底避免了系统级 malloc/free 带来的碎片和锁开销
	
	Block：真正给你的内存块。它只有一个指针 next。
	注意：当这块内存在“空闲”状态时，这个指针用于串联链表；当这块内存被“分配”后，这个位置会被数据覆盖，完全不影响使用。

	Chunk：一个16KB的大块内存（称为“块堆”）。
	blockSize：该大块被切成多大多小的块
	blocks：指向这一大块内存的首地址，初始化时每一个Block有一个指针next指向下一个Block，最后一个Block的next为空指针
	
	构造函数初始化内存池
	块堆数组 的 占用空间大小 m_ChunkSpace 增加 块堆数组增量128
	块堆数组 中 块堆的数量 m_ChunkCount 置零
	分配 块堆数组的内存空间 m_Chunks（空间大小为m_ChunkSpace * sizeof(Chunk)）
	块堆数组的内存空间 m_Chunks 置零
	将空闲块指针列表 m_FreeLists 置零

	当你调用 Allocate(size)（例如 size = 20）
	1、越界处理：若 size > 640，直接调用 CustomAlloc 交给系统堆（new），这类大对象不走内存池。
	2、映射索引：查表 index = SizeMapInstance.values[20] = 1，确定要使用 32B 的规格。
	3、尝试取空闲块：
	  检查 m_FreeLists[1]。如果不为空，将块链表头部的 空闲块地址 返回给用户，将m_FreeLists[1]设置为m_FreeLists[1].next，即下一个块的地址。
	  无空闲块，申请新Chunk：
	    （若m_FreeLists[1]为空，意味着该规格的“货架”卖空了，需要补货）
		取块堆数组 的 第 m_ChunkCount 个块堆，即最后一个块堆的下一个块堆
	    给新块堆 调用 CustomAlloc(ChunkSize) 申请一块 16KB 的原始内存。
	    计算 blockCount = 16KB / 32B = 512 个块。
	    将新块堆的 16KB 内存初始化成一个链表：第0块的next指针指向第1块，第1块的next指针指向第2块……最后一块指向 nullptr。
	    将 m_FreeLists[1] 设置为第1块，将第0块返回给用户

    动态扩容机制（块堆数组）
	m_Chunks 是一个存储 Chunk* 元数据的数组，而不是16KB大内存本身。
	初始容量：128 个 Chunk 指针。
	扩容触发：当 m_ChunkCount == m_ChunkSpace（存满了）。
	扩容动作：
	m_ChunkSpace += 128。
	CustomAlloc 申请 m_ChunkSpace 尺寸的新块堆数组。
	memcpy 旧数组的 Chunk 元数据（注意：拷贝的是指针，16KB大内存纹丝不动！）。
	memset 新扩展部分为0。
	CustomFree 旧元数据数组。

	释放(Free) 
	1、过滤：空指针和 size 为 0 直接空操作，size > 640 则交给系统堆释放 CustomFree。
	2、获取索引：查表 index = SizeMapInstance.values[20] = 1，确定为 32B 的规格
	3、Debug 校验（极重要）：
	  遍历 m_Chunks 数组，确认被释放的指针 p 确实落在某个Chunk的管理范围内。
	  同时检查该 Chunk 的 blockSize 是否与传入的 size 匹配。
	  这是防止外部传入野指针或错位释放的“安全闸门”。（Release版本通常关闭）。
	4、回收链表：
	  将 p 强制转为 Block*，将其 next 指向当前空闲链表头 m_FreeLists[index]。
	  更新 m_FreeLists[index] = p。这就是头插法（LIFO），也就是将p地址设置成空闲块链表的表头 m_FreeLists[index]。

	*/


	const uint8_t BlockSizeCount = 14;

	struct Chunk;
	struct Block;

	class BlockAllocator
	{
	public:
		~BlockAllocator();

		static std::shared_ptr<BlockAllocator> GetInstance() 
		{
			std::call_once(init_flag, []() {
				instance = std::shared_ptr<BlockAllocator>(new BlockAllocator());
				});
			return instance;
		}

		/*
		根据SizeMap分配内存，返回一个内存块block的指针。
		size=0~16, 分配16B内存；
		size=17~32，分配32B的内存；
		size=33~64，分配64B的内存；
		……
		size > MaxBlockSize = 640，使用Alloc。
		*/
		void* Allocate(size_t size);

		// 释放内存。如果size > MaxBlockSize，调用CustomFree
		void Free(void* p, size_t size);

		void Clear();

	private:
		BlockAllocator();                                          // 私有构造函数，用于单例模式
		BlockAllocator(BlockAllocator const&) = delete;            // 禁止拷贝构造
		BlockAllocator(BlockAllocator &&) = delete;                // 禁止移动构造
		BlockAllocator& operator=(BlockAllocator const&) = delete; // 禁止赋值操作
		BlockAllocator& operator=(BlockAllocator &&) = delete;     // 禁止移动操作

	private:
		Chunk* m_Chunks;        // 块堆数组
		size_t m_ChunkCount;    // 块堆数量
		size_t m_ChunkSpace;    // 块堆数组占用多少个块堆的内存空间

		// 空闲块指针表，索引0~13，指向从16B到640B的各尺寸的空闲块，每个元素是一个链表头，指向对应尺寸的第一个空闲块 Block
		Block* m_FreeLists[BlockSizeCount];

		static std::shared_ptr<BlockAllocator> instance;
		static std::once_flag init_flag;
	};

}