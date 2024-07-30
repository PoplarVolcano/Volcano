#include "volpch.h"
#include <Volcano.h>
#include "imgui/imgui.h"
/*
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
glm::mat4 camera(float Translate, glm::vec2 const& Rotate)
{
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.f);
	glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Translate));
	View = glm::rotate(View, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
	View = glm::rotate(View, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	return Projection * View * Model;
}
*/
class ExampleLayer : public Volcano::Layer {
public:
	ExampleLayer() : Layer("Example"){
		//auto cam = camera(5.0f, { 0.5f, 0.5f });
	}

	void OnUpdate() override {
		//VOL_INFO("ExampleLayer::Update");
		if (Volcano::Input::IsKeyPressed(VOL_KEY_TAB))
			VOL_TRACE("Tab key is pressed!");
	}

	void OnImGuiRender() override {
		ImGui::Begin("Test");
		ImGui::Text("Test");
		ImGui::End();
	}

	void OnEvent(Volcano::Event& event) override {
		//VOL_TRACE("{0}", event);
		if (event.GetEventType() == Volcano::EventType::KeyPressed) {
			Volcano::KeyPressedEvent& e = (Volcano::KeyPressedEvent&)event;
			VOL_TRACE("{0}", (char)e.GetKeyCode());
		}
		if (event.GetEventType() == Volcano::EventType::MouseButtonPressed) {
			Volcano::MouseButtonEvent& e = (Volcano::MouseButtonEvent&)event;
			VOL_TRACE("{0}", e.GetMouseButton());
		}
		if (event.GetEventType() == Volcano::EventType::MouseScrolled) {
			Volcano::MouseScrolledEvent& e = (Volcano::MouseScrolledEvent&)event;
			VOL_TRACE("{0}", e.GetXOffset());
			VOL_TRACE("{0}", e.GetYOffset());
		}
		if (event.GetEventType() == Volcano::EventType::MouseMoved) {
			Volcano::MouseMovedEvent& e = (Volcano::MouseMovedEvent&)event;
			//VOL_TRACE("{0}", e.GetX());
			//VOL_TRACE("{0}", e.GetY());
		}
	}

};

class Sandbox : public Volcano::Application {
public:
	Sandbox() {
		PushLayer(new ExampleLayer());
	}

	~Sandbox() {
	}

};

//创建应用
Volcano::Application* Volcano::CreateApplication() {
	return new Sandbox;
}
