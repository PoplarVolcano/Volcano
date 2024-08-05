#include "volpch.h"
#include <Volcano.h>
#include "Platform/OpenGL/OpenGLShader.h"
#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	ExampleLayer() 
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		m_VertexArray.reset(Volcano::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};

		Volcano::Ref<Volcano::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Volcano::VertexBuffer::Create(vertices, sizeof(vertices)));

		Volcano::BufferLayout layout = {
			{ Volcano::ShaderDataType::Float3, "a_Position" },
			{ Volcano::ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		Volcano::Ref<Volcano::IndexBuffer> indexBuffer;
		indexBuffer.reset(Volcano::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		//着色器
		//顶点布局
		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;
			
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;

			void main(){
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4( a_Position,1.0);
			}
		)";
		//绘制颜色
		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main(){
				color = vec4(v_Position*0.5+0.5,1.0);
				color = v_Color;
			}
		)";
		m_Shader = Volcano::Shader::Create("VertexColorTriangle", vertexSrc, fragmentSrc);
		//shader

		m_SquareVA.reset(Volcano::VertexArray::Create());

		float squareVertices[4 * 5] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Volcano::Ref<Volcano::VertexBuffer> squareVB;
		squareVB.reset(Volcano::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		squareVB->SetLayout({
			{ Volcano::ShaderDataType::Float3, "a_Position" },
			{ Volcano::ShaderDataType::Float2, "a_TexCoord" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Volcano::Ref<Volcano::IndexBuffer> squareIB;
		squareIB.reset(Volcano::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);


		std::string flatColorShaderVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main(){
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4( a_Position,1.0);
			}
		)";
		//绘制颜色
		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			uniform vec3 u_Color;

			void main(){
				color = vec4(u_Color, 1.0);
			}
		)";
		m_FlatColorShader = Volcano::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);


		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = Volcano::Texture2D::Create("assets/textures/15 (4).jpg");
		m_AlterTexture = Volcano::Texture2D::Create("assets/textures/莫斯提马.png");

		std::dynamic_pointer_cast<Volcano::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Volcano::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);

	}

	void OnUpdate(Volcano::Timestep ts) override 
	{
		//VOL_TRACE("Delta time: {0}s, ({1}ms)", ts, ts.GetMilliseconds());
		if (Volcano::Input::IsKeyPressed(VOL_KEY_LEFT))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		if (Volcano::Input::IsKeyPressed(VOL_KEY_RIGHT))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;
		if (Volcano::Input::IsKeyPressed(VOL_KEY_DOWN))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;
		if (Volcano::Input::IsKeyPressed(VOL_KEY_UP))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;

		if (Volcano::Input::IsKeyPressed(VOL_KEY_A))
			m_CameraRotation += m_CameraRotationSpeed * ts;
		if (Volcano::Input::IsKeyPressed(VOL_KEY_D))
			m_CameraRotation -= m_CameraRotationSpeed * ts;

		Volcano::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Volcano::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Volcano::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<Volcano::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Volcano::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);
		
		for(int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Volcano::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}

		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();
		Volcano::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		m_AlterTexture->Bind();
		Volcano::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		//Triangle
		//Volcano::Renderer::Submit(m_Shader, m_VertexArray);

		Volcano::Renderer::EndScene();

	}

	void OnImGuiRender() override 
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	void OnEvent(Volcano::Event& event) override {
		//VOL_TRACE("{0}", event);
		/*
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
		*/


	}
private:

	Volcano::ShaderLibrary m_ShaderLibrary;
	Volcano::Ref<Volcano::Shader> m_Shader;
	Volcano::Ref<Volcano::VertexArray> m_VertexArray;

	Volcano::Ref<Volcano::Shader> m_FlatColorShader;
	Volcano::Ref<Volcano::VertexArray> m_SquareVA;

	Volcano::Ref<Volcano::Texture2D> m_Texture, m_AlterTexture;

	Volcano::OrthoGraohicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 5.0f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 180.0f;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.7f };
};

class Sandbox : public Volcano::Application {
public:
	Sandbox() 
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox() {
	}

};

//创建应用
Volcano::Application* Volcano::CreateApplication() {
	return new Sandbox;
}
