#pragma once

#ifdef VOL_PLATFORM_WINDOWS

//���η�extern���ڱ������ߺ���������ǰ������˵�����˱���/�������ڱ𴦶���ģ�Ҫ�ڴ˴����á�
extern Volcano::Application* Volcano::CreateApplication();

int main(int argc, char** argv)
{
	Volcano::InitializeCore();
	Volcano::Application* app = Volcano::CreateApplication();
	VOL_CORE_ASSERT(app, "Client Application is null!");
	app->Run();
	delete app;
	Volcano::ShutdownCore();

	return 0;
}

#endif