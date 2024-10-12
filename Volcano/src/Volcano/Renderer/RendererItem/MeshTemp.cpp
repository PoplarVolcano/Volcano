#include "volpch.h"
#include "MeshTemp.h"

#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Scene/Entity.h"

namespace Volcano {

    Ref<Texture2D> MeshTemp::m_WhiteTextures[2];
    Ref<Texture2D> MeshTemp::m_BlackTextures[5];

    MeshTemp::MeshTemp(std::vector<MeshTempVertex> vertices, std::vector<uint32_t> indices)
    {
        m_VertexSize = vertices.size();
        m_IndexSize = indices.size();
        MaxMeshes   = 1;
        MaxVertices = MaxMeshes * m_VertexSize;
        MaxIndices  = MaxMeshes * m_IndexSize;
        m_Vertices  = vertices;
        m_Indices   = indices;
        SetupMesh();
    }

    void MeshTemp::Init()
    {
        for (uint32_t i = 0; i < 2; i++)
        {
            Ref<Texture2D> whiteTexture = Texture2D::Create(1, 1);
            uint32_t whiteTextureData = 0xffffffff;
            whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
            m_WhiteTextures[i] = whiteTexture;
        }

        for (uint32_t i = 0; i < 5; i++)
        {
            Ref<Texture2D> m_BlackTexture = Texture2D::Create(1, 1);
            uint32_t m_BlackTextureData = 0x00000000;
            m_BlackTexture->SetData(&m_BlackTextureData, sizeof(uint32_t));
            m_BlackTextures[i] = m_BlackTexture;
        }
    }

    void MeshTemp::Shutdown()
    {
        delete[] vertexBufferBase;
    }

    void MeshTemp::BeginScene(const Camera& camera, const glm::mat4& transform, const glm::vec3& position)
    {
        float Shininess = 32.0f;
        UniformBufferManager::GetUniformBuffer("Material")->SetData(&Shininess, sizeof(float));

        //StartBatch();
    }

    void MeshTemp::EndScene()
    {
        Flush();
    }

    void MeshTemp::Flush()
    {
        if (m_IndexCount)
        {
            // 不加uint8_t转化会得到元素数量, 加uint8_t返回以char为单位占用多少元素
            uint32_t dataSize = (uint32_t)((uint8_t*)vertexBufferPtr - (uint8_t*)vertexBufferBase);
            m_VertexBuffer->SetData(vertexBufferBase, dataSize);

            Draw();
        }
    }

    void MeshTemp::StartBatch()
    {
        m_IndexCount = 0;
        vertexBufferPtr = vertexBufferBase;
    }

    void MeshTemp::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void MeshTemp::DrawMesh(int entityID)
	{
        if (m_IndexCount >= MaxIndices)
            NextBatch();

        glm::mat4 transform = m_Entity->GetTransform();
        Entity* entityParent = m_Entity->GetEntityParent();
        while (entityParent != nullptr)
        {
            transform =  entityParent->GetTransform() * transform;
            entityParent = entityParent->GetEntityParent();
        }
        glm::mat3 normalTransform = glm::mat3(transpose(inverse(transform)));

        for (uint32_t i = 0; i < m_VertexSize; i++)
        {
            vertexBufferPtr->Position  = transform * glm::vec4(m_Vertices[i].Position, 1.0f);
            vertexBufferPtr->TexCoords = m_Vertices[i].TexCoords;
            vertexBufferPtr->Normal    = normalTransform * m_Vertices[i].Normal;
            vertexBufferPtr->Tangent   = normalTransform * m_Vertices[i].Tangent;
            vertexBufferPtr->Bitangent = glm::cross(vertexBufferPtr->Normal, vertexBufferPtr->Tangent);
            vertexBufferPtr->EntityID  = entityID;
            vertexBufferPtr++;
        }
        m_IndexCount += m_IndexSize;
	}

    void MeshTemp::Draw()
    {
        Renderer::DrawIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());
    }

    void MeshTemp::BindTextures(std::vector<std::pair<ImageType, Ref<Texture>>> textures)
    {
        std::unordered_map<ImageType, Ref<Texture>> texturesTemp;
        for (auto& [type, texture] : textures)
            texturesTemp[type] = texture;

        if (texturesTemp[ImageType::Diffuse])
            texturesTemp[ImageType::Diffuse]->Bind(0);
        else
            m_WhiteTextures[0]->Bind(0);

        if (texturesTemp[ImageType::Specular])
            texturesTemp[ImageType::Specular]->Bind(1);
        else
            m_BlackTextures[0]->Bind(1);

        if (texturesTemp[ImageType::Normal])
            texturesTemp[ImageType::Normal]->Bind(2);
        else
            m_BlackTextures[1]->Bind(2);

        if (texturesTemp[ImageType::Height])
            texturesTemp[ImageType::Height]->Bind(3);
        else
            m_BlackTextures[2]->Bind(3);

        if (texturesTemp[ImageType::Roughness])
            texturesTemp[ImageType::Roughness]->Bind(4);
        else
            m_BlackTextures[3]->Bind(4);

        if (texturesTemp[ImageType::AO])
            texturesTemp[ImageType::AO]->Bind(5);
        else
            m_BlackTextures[4]->Bind(5);

    }

    void MeshTemp::BindShader(RenderType type)
    {
        switch (type)
        {
        case RenderType::SHADOW_DIRECTIONALLIGHT:
            Renderer::GetShaderLibrary()->Get("ShadowMappingDepth")->Bind();
            break;
        case RenderType::SHADOW_POINTLIGHT:
            Renderer::GetShaderLibrary()->Get("PointShadowsDepth")->Bind();
            break;
        case RenderType::SHADOW_SPOTLIGHT:
            Renderer::GetShaderLibrary()->Get("SpotShadowDepth")->Bind();
            break;
        case RenderType::G_BUFFER:
            Renderer::GetShaderLibrary()->Get("GBuffer")->Bind();
            break;
        case RenderType::PBRLIGHT_SHADING:
            Renderer::GetShaderLibrary()->Get("PBRLightShading")->Bind();
            break;
        case RenderType::LIGHT_SHADING:
            Renderer::GetShaderLibrary()->Get("LightShading")->Bind();
            break;
        case RenderType::PBRDEFERRED_SHADING:
            Renderer::GetShaderLibrary()->Get("PBRDeferredShading")->Bind();
            break;
        case RenderType::DEFERRED_SHADING:
            Renderer::GetShaderLibrary()->Get("DeferredShading")->Bind();
            break;
        case RenderType::NORMAL:
            break;
        default:
            VOL_CORE_ASSERT(0);
        }
    }

    void MeshTemp::ResetVertexBufferBase()
    {
        vertexBufferBase = new MeshTempVertex[MaxVertices];
    }

    void MeshTemp::SetupMesh()
    {
        m_VertexArray = VertexArray::Create();
        //vb = VertexBuffer::Create((void*)&vertices[0], vertices.size() * sizeof(MeshVertex));
        m_VertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(MeshTempVertex));
        m_VertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position"        },
            { ShaderDataType::Float2, "a_TexCoords"       },
            { ShaderDataType::Float3, "a_Normal"          },
            { ShaderDataType::Float3, "a_Tangent"         },
            { ShaderDataType::Float3, "a_Bitangent"       },
            { ShaderDataType::Int,    "a_EntityID"        }
            });
        m_VertexArray->AddVertexBuffer(m_VertexBuffer);
        vertexBufferBase = new MeshTempVertex[MaxVertices];

        uint32_t* indicesBuffer = new uint32_t[MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < MaxIndices; i += m_IndexSize)
        {
            for (uint32_t j = 0; j < m_IndexSize; j++)
                indicesBuffer[i + j] = offset + m_Indices[j];
            offset += 36; // +vertexSize
        }
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indicesBuffer, MaxIndices);;
        m_VertexArray->SetIndexBuffer(indexBuffer);
        delete[] indicesBuffer;	// cpu上传到gpu上了可以删除cpu的索引数据块了

    }

}