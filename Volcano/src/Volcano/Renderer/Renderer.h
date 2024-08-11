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

		//void* p1 = (void*)ptr; 参数，方法的指针，表示把整型指针进行强制类型转换成空类型指针
		template<typename FuncT>
		static void Submit(FuncT&& func)
		{
			//构建渲染指令，lambda表达式，参数:方法指针
			auto renderCmd = [](void* ptr) {
				//函数指针pFunc
				auto pFunc = (FuncT*)ptr;
				//执行方法，通过函数指针pFunc调用一个函数
				(*pFunc)();

				pFunc->~FuncT();
				};
			//把渲染指令加入指令队列
			auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
			// std::forward有一个用例：
			// 将模板化的函数参数（在函数内部）转换为用于传递它的调用方的值类别（左值或右值）。
			// 这允许将右值参数作为右值传递，并将左值作为左值传递，这是一种称为“完美转发”的方案。
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
		}
		static void WaitAndRender();
	private:
		static RenderCommandQueue& GetRenderCommandQueue();
	};
}