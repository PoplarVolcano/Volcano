#pragma once

#include "Volcano/Renderer/RenderCommandQueue.h"
#include "Volcano/Renderer/RendererAPI.h"
#include "Volcano/Renderer/Shader.h"

namespace Volcano {

	class Renderer
	{
	public:
		typedef void(*RenderCommandFn)(void*);

		static void Init();
		static void OnWindowResize(uint32_t width, uint32_t height);

		// Commands
		static void Clear();
		static void Clear(float r, float g, float b, float a = 1.0f);
		static void SetClearColor(float r, float g, float b, float a);

		static void DrawIndexed(PrimitiveType type, uint32_t count, bool depthTest = true);

		static const Scope<ShaderLibrary>& GetShaderLibrary();

		//void* p1 = (void*)ptr; ������������ָ�룬��ʾ������ָ�����ǿ������ת���ɿ�����ָ��
		template<typename FuncT>
		static void Submit(FuncT&& func)
		{
			//������Ⱦָ�lambda���ʽ������:����ָ��
			auto renderCmd = [](void* ptr) {
				//����ָ��pFunc
				auto pFunc = (FuncT*)ptr;
				//ִ�з�����ͨ������ָ��pFunc����һ������
				(*pFunc)();

				pFunc->~FuncT();
				};
			//����Ⱦָ�����ָ�����
			auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
			// std::forward��һ��������
			// ��ģ�廯�ĺ����������ں����ڲ���ת��Ϊ���ڴ������ĵ��÷���ֵ�����ֵ����ֵ����
			// ��������ֵ������Ϊ��ֵ���ݣ�������ֵ��Ϊ��ֵ���ݣ�����һ�ֳ�Ϊ������ת�����ķ�����
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
		}
		static void WaitAndRender();
	private:
		static RenderCommandQueue& GetRenderCommandQueue();
	};
}