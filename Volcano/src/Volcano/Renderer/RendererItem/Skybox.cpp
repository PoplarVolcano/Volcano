#include "volpch.h"
#include "Skybox.h"
#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

    struct SkyboxData
    {
        Ref<VertexArray> VertexArray;
        Ref<Shader> Shader;
        Ref<TextureCube> Texture;
    };
    static SkyboxData s_SkyboxData;

	void Skybox::Init()
	{
        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
        uint32_t indices[36];
        for (uint32_t i = 0; i < 36; i++)
            indices[i] = i;
        s_SkyboxData.VertexArray = VertexArray::Create();
        Ref<VertexBuffer> vb = VertexBuffer::Create(skyboxVertices, sizeof(skyboxVertices));
        vb->SetLayout({
            { ShaderDataType::Float3, "a_Position"}
            });
        s_SkyboxData.VertexArray->AddVertexBuffer(vb);

        Ref<IndexBuffer> ib = IndexBuffer::Create(indices, 36);
        s_SkyboxData.VertexArray->SetIndexBuffer(ib);
        Renderer::GetShaderLibrary()->Load("assets/shaders/3D/Skybox.glsl");
        s_SkyboxData.Shader = Renderer::GetShaderLibrary()->Get("Skybox");
        //s_SkyboxData.Texture = TextureCube::Create("SandBoxProject/Assets/Textures/skybox_texture.jpg");
        std::vector<std::string> faces
        {
            std::string("SandBoxProject/Assets/Textures/skybox/right.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/left.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/top.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/bottom.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/front.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/back.jpg"),
        };
        s_SkyboxData.Texture = TextureCube::Create(faces);

        s_CameraDataUniformBuffer = UniformBuffer::Create(4 * 4 * 2 * sizeof(float), 7);
	}

    void Skybox::Shutdown()
    {
    }

    void Skybox::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        s_CameraDataBuffer.View = glm::inverse(transform);
        s_CameraDataBuffer.View = glm::mat4(glm::mat3(s_CameraDataBuffer.View));
        s_CameraDataBuffer.Projection = camera.GetProjection();
        s_CameraDataUniformBuffer->SetData(&s_CameraDataBuffer.View,       sizeof(glm::mat4));
        s_CameraDataUniformBuffer->SetData(&s_CameraDataBuffer.Projection, sizeof(glm::mat4), (4 * 4) * sizeof(float));
    }

    void Skybox::EndScene()
    {
    }

    void Skybox::DrawSkybox()
    {
        s_SkyboxData.Texture->Bind();
        s_SkyboxData.Shader->Bind();
        Renderer::DrawIndexed(s_SkyboxData.VertexArray, s_SkyboxData.VertexArray->GetIndexBuffer()->GetCount());

    }

    void Skybox::SetTexture(Ref<TextureCube> texture)
    {
        s_SkyboxData.Texture = texture;
    }

}
