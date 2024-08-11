#pragma once

#include "volpch.h"

namespace Volcano {

	class RenderCommandQueue
	{
	public:
		//渲染指令方法
		typedef void(*RenderCommandFn)(void*);

		RenderCommandQueue();
		~RenderCommandQueue();

		//分配
		void* Allocate(RenderCommandFn func, uint32_t size);

		//执行
		void Execute();
	private:
		//指令缓冲
		uint8_t* m_CommandBuffer;
		//指令缓冲指针
		uint8_t* m_CommandBufferPtr;
		//指令计数
		uint32_t m_CommandCount = 0;
	};



}