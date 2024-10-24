#include "volpch.h"
#include "Skybox.h"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Utils/PlatformUtils.h"

namespace Volcano {

    Ref<SkyboxData> Skybox::m_SkyboxData;

    void Skybox::Init()
	{
        m_SkyboxData = std::make_shared<SkyboxData>();

        float vertices[] = {
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
        m_SkyboxData->VertexArray = VertexArray::Create();
        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position"}
            });
        m_SkyboxData->VertexArray->AddVertexBuffer(vertexBuffer);

        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, 36);
        m_SkyboxData->VertexArray->SetIndexBuffer(indexBuffer);
        m_SkyboxData->Shader = Renderer::GetShaderLibrary()->Get("Skybox");

        m_SkyboxData->Texture = TextureCube::Create(TextureFormat::RGB16F, 512, 512);
        
        /*
        std::vector<std::string> faces
        {
            (FileDialogs::GetProjectPath() / "SandBoxProject/Assets/Textures/skybox/right.jpg").string(),
            (FileDialogs::GetProjectPath() / "SandBoxProject/Assets/Textures/skybox/left.jpg").string(),
            (FileDialogs::GetProjectPath() / "SandBoxProject/Assets/Textures/skybox/top.jpg").string(),
            (FileDialogs::GetProjectPath() / "SandBoxProject/Assets/Textures/skybox/bottom.jpg").string(),
            (FileDialogs::GetProjectPath() / "SandBoxProject/Assets/Textures/skybox/front.jpg").string(),
            (FileDialogs::GetProjectPath() / "SandBoxProject/Assets/Textures/skybox/back.jpg").string()
        };
        m_SkyboxData->Texture = TextureCube::Create(faces);
        */

	}

    void Skybox::Shutdown()
    {
    }

    void Skybox::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        s_CameraDataBuffer.View = glm::inverse(transform);
        s_CameraDataBuffer.Projection = camera.GetProjection();
        UniformBufferManager::GetUniformBuffer("CameraData")->SetData(&s_CameraDataBuffer.View, sizeof(glm::mat4));
        UniformBufferManager::GetUniformBuffer("CameraData")->SetData(&s_CameraDataBuffer.Projection, sizeof(glm::mat4), (4 * 4) * sizeof(float));
    }

    void Skybox::EndScene()
    {
    }

    void Skybox::DrawSkybox()
    {
        if (m_SkyboxData->Texture == nullptr) 
            m_SkyboxData->Texture = TextureCube::Create(TextureFormat::RGB16F, 512, 512);
        m_SkyboxData->Texture->Bind();
        m_SkyboxData->Shader->Bind();
        DrawIndexed();
    }

    void Skybox::DrawIndexed()
    {
        Renderer::DrawIndexed(m_SkyboxData->VertexArray, m_SkyboxData->VertexArray->GetIndexBuffer()->GetCount());
    }

    void Skybox::SetTexture(Ref<TextureCube> texture)
    {
        m_SkyboxData->Texture = texture;
    }

}
