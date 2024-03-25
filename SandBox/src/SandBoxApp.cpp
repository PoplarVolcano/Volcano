#include "volpch.h"
#include <Volcano.h>

class ExampleLayer : public Volcano::Layer {
public:
	ExampleLayer() : Layer("Example"){}

	void OnUpdate() override {
		VOL_INFO("ExampleLayer::Update");
	}

	void OnEvent(Volcano::Event& event) override {
		VOL_TRACE("{0}", event);
	}

};

class Sandbox : public Volcano::Application {
public:
	Sandbox() {
		PushLayer(new ExampleLayer());
		PushOverlay(new Volcano::ImGuiLayer());
	}

	~Sandbox() {
	}

};

Volcano::Application* Volcano::CreateApplication() {
	return new Sandbox;
}