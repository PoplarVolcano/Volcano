#pragma once

#include "volpch.h"

namespace Volcano {

	class RenderCommandQueue
	{
	public:
		//��Ⱦָ���
		typedef void(*RenderCommandFn)(void*);

		RenderCommandQueue();
		~RenderCommandQueue();

		//����
		void* Allocate(RenderCommandFn func, uint32_t size);

		//ִ��
		void Execute();
	private:
		//ָ���
		uint8_t* m_CommandBuffer;
		//ָ���ָ��
		uint8_t* m_CommandBufferPtr;
		//ָ�����
		uint32_t m_CommandCount = 0;
	};



}