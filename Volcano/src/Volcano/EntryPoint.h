#pragma once

#ifdef VOL_PLATFORM_WINDOWS

//修饰符extern用在变量或者函数的声明前，用来说明“此变量/函数是在别处定义的，要在此处引用”
extern Volcano::Application* Volcano::CreateApplication();

int main(int argc, char** argv) 
{
	//初始化日志
	Volcano::Log::Init();
	VOL_CORE_WARN("Initialized Log!");
	int a = 5;
	VOL_INFO("Hello! Var={0}", a);

	auto app = Volcano::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#endif