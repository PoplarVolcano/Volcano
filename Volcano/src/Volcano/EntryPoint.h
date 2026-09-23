#pragma once

#ifdef VOL_PLATFORM_WINDOWS

//修饰符extern用在变量或者函数的声明前，用来说明“此变量/函数是在别处定义的，要在此处引用”
extern Volcano::Application* Volcano::CreateApplication();

//程序启动点
int main(int argc, char** argv)
{
	Volcano::InitializeCore();
	Volcano::Application* app = Volcano::CreateApplication();
	app->Run();
	delete app;
	Volcano::ShutdownCore();

	return 0;
}

#endif