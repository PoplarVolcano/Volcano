#pragma once

#ifdef VOL_PLATFORM_WINDOWS

extern Volcano::Application* Volcano::CreateApplication();

int main(int argc, char** argv) {
	auto app = Volcano::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#endif