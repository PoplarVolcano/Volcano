#include <Volcano.h>

class Sandbox : public Volcano::Application {
public:
	Sandbox() {

	}

	~Sandbox() {

	}

};

Volcano::Application* Volcano::CreateApplication() {
	return new Sandbox;
}