#pragma once

#ifdef VOL_PLATFORM_WINDOWS

//���η�extern���ڱ������ߺ���������ǰ������˵�����˱���/�������ڱ𴦶���ģ�Ҫ�ڴ˴����á�
extern Volcano::Application* Volcano::CreateApplication();

int main(int argc, char** argv) 
{
	//��ʼ����־
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