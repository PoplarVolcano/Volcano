#include "ExampleLayer.h"
#include <glm/ext/matrix_transform.hpp>
#include "Volcano/Renderer/Renderer2D.h"

namespace Volcano{

    ExampleLayer::ExampleLayer()
        : m_CameraController(1280.0f / 720.0f, true)
    {
    }

    ExampleLayer::~ExampleLayer()
    {
    }

    void ExampleLayer::OnAttach()
    {
        m_Texture = Texture2D::Create("assets/textures/Mostima.png");
        m_AlterTexture = Texture2D::Create("assets/textures/ÄªË¹ÌáÂí.png");
    }

    void ExampleLayer::OnDetach()
    {
    }

    void ExampleLayer::OnUpdate(Timestep ts)
    {
        m_CameraController.OnUpdate(ts);
        Renderer::Clear(0.1f, 0.1f, 0.1f, 1);

        Renderer2D::BeginScene(m_CameraController.GetCamera());
        {
            static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
            glm::vec4  redColor(0.8f, 0.3f, 0.3f, 1.0f);
            glm::vec4  blueColor(0.2f, 0.3f, 0.8f, 1.0f);

            Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_Texture);
            Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_AlterTexture);
            Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, redColor);
            Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, blueColor);

            Renderer2D::EndScene();

        }
    }

    void ExampleLayer::OnImGuiRender()
    {
    }

    void ExampleLayer::OnEvent(Event& event)
    {
        m_CameraController.OnEvent(event);
    }
}