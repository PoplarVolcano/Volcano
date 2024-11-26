#include "volpch.h"
#include "Mesh.h"

#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Scene/Entity.h"
#include "Volcano/Renderer/RendererItem/Model.h"
#include "Volcano/Renderer/RendererItem/Animator.h"
#include "Volcano/Scene/SceneSerializer.h"
#include "Volcano/Project/Project.h"

namespace Volcano {

    Ref<Texture2D> Mesh::m_WhiteTextures[2];
    Ref<Texture2D> Mesh::m_BlackTextures[5];

    std::once_flag Mesh::init_flag;
    Scope<MeshLibrary> Mesh::m_MeshLibrary;


    void Mesh::Init()
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

    void Mesh::Shutdown()
    {
        delete[] vertexBufferBase;
    }

    void Mesh::BeginScene(const Camera& camera, const glm::mat4& transform, const glm::vec3& position)
    {
        float Shininess = 32.0f;
        UniformBufferManager::GetUniformBuffer("Material")->SetData(&Shininess, sizeof(float));

        
        //StartBatch();
    }

    void Mesh::EndScene()
    {
        Flush();
    }

    void Mesh::Flush()
    {
        if (m_IndexCount)
        {
            // 不加uint8_t转化会得到元素数量, 加uint8_t返回以char为单位占用多少元素
            uint32_t dataSize = (uint32_t)((uint8_t*)vertexBufferPtr - (uint8_t*)vertexBufferBase);
            m_VertexBuffer->SetData(vertexBufferBase, dataSize);

            glm::mat4 transform = m_Entity->GetParentTransform() * m_Entity->GetTransform();
            glm::mat4 normalTransform = transpose(inverse(transform));//glm::mat3(transpose(inverse(transform)));

            UniformBufferManager::GetUniformBuffer("ModelTransform")->SetData(&transform, sizeof(glm::mat4));
            UniformBufferManager::GetUniformBuffer("ModelTransform")->SetData(&normalTransform, sizeof(glm::mat4), 4 * 4 * sizeof(float));


            auto& mc = m_Entity->GetComponent<MeshComponent>();
            if (mc.meshType == MeshType::Model || !mc.vertexBone.empty())
            {
                Entity* entityTemp = m_Entity;
                do
                {
                    if (entityTemp->HasComponent<AnimationComponent>() && entityTemp->HasComponent<AnimatorComponent>())
                    {

                        auto& animatorComponent = entityTemp->GetComponent<AnimatorComponent>();
                        auto& finalBoneMatrices = animatorComponent.animator->GetFinalBoneMatrices();
                        for(uint32_t i = 0; i < finalBoneMatrices.size(); i++)
                            UniformBufferManager::GetUniformBuffer("BonesMatrices")->SetData(&finalBoneMatrices[i], sizeof(glm::mat4), i * 4 * 4 * sizeof(float));
                            //UniformBufferManager::GetUniformBuffer("BonesMatrices")->SetData(&finalBoneMatrices, finalBoneMatrices.size() * 4 * 4 * sizeof(float));
                        break;
                    }
                    else
                        entityTemp = entityTemp->GetEntityParent();
                } while (entityTemp != nullptr);
                if (entityTemp == nullptr) 
                {
                    float zero[1600] = { 0.0f };
                    UniformBufferManager::GetUniformBuffer("BonesMatrices")->SetData(&zero, sizeof(zero));
                }

            }
            else
            {
                float zero[1600] = { 0.0f };
                UniformBufferManager::GetUniformBuffer("BonesMatrices")->SetData(&zero, sizeof(zero));
            }


            Draw();
        }
    }

    void Mesh::StartBatch()
    {
        m_IndexCount = 0;
        vertexBufferPtr = vertexBufferBase;
    }

    void Mesh::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Mesh::DrawMesh(int entityID, std::vector<glm::mat4>& finalBoneMatrices)
	{
        if (m_IndexCount >= MaxIndices)
            NextBatch();

        for (uint32_t i = 0; i < m_VertexSize; i++)
        {
            vertexBufferPtr->Position  = m_Vertices[i].Position;
            vertexBufferPtr->TexCoords = m_Vertices[i].TexCoords;
            vertexBufferPtr->Normal    = m_Vertices[i].Normal;
            vertexBufferPtr->Tangent   = m_Vertices[i].Tangent;
            vertexBufferPtr->Bitangent = m_Vertices[i].Bitangent;
            for (uint32_t index = 0; index < MAX_BONE_INFLUENCE; index++)
            {
                vertexBufferPtr->BoneIDs[index] = m_Vertices[i].BoneIDs[index];
                vertexBufferPtr->Weights[index] = m_Vertices[i].Weights[index];
            }

            /*
            if (!finalBoneMatrices.empty())
            {
                glm::vec4 position = glm::vec4(0.0f);
                for (uint32_t index = 0; index < MAX_BONE_INFLUENCE; index++)
                {
                    vertexBufferPtr->BoneIDs[index] = m_Vertices[i].BoneIDs[index];
                    vertexBufferPtr->Weights[index] = m_Vertices[i].Weights[index];

                    if (vertexBufferPtr->BoneIDs[index] == -1)
                        continue;
                    if (vertexBufferPtr->BoneIDs[index] >= 100)
                    {
                        position = glm::vec4(m_Vertices[i].Position, 1.0f);
                        break;
                    }
                    glm::vec4 localPosition = finalBoneMatrices[vertexBufferPtr->BoneIDs[index]] * glm::vec4(m_Vertices[i].Position, 1.0f);
                    position += localPosition * vertexBufferPtr->Weights[index];
                }
                //vertexBufferPtr->Position = transform * position;
                vertexBufferPtr->Position = position;
            }
            else
                //vertexBufferPtr->Position = transform * glm::vec4(m_Vertices[i].Position, 1.0f);
                vertexBufferPtr->Position = m_Vertices[i].Position;
            */
            vertexBufferPtr->EntityID  = entityID;
            vertexBufferPtr++;
        }
        m_IndexCount += m_IndexSize;
	}

    void Mesh::Draw()
    {
        Renderer::DrawIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());
    }

    void Mesh::BindTextures(std::vector<std::pair<ImageType, Ref<Texture>>> textures)
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

    void Mesh::BindShader(RenderType type)
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
        case RenderType::COLLIDER:
            Renderer::GetShaderLibrary()->Get("Collider")->Bind();
            break;
        case RenderType::NORMAL:
            break;
        default:
            VOL_CORE_ASSERT(0);
        }
    }

    void Mesh::ResetVertexBufferBase()
    {
        vertexBufferBase = new MeshVertex[MaxVertices];
    }

    void Mesh::SetBoneID(int vertexIndex1, int vertexIndex2, int boneIndex, float weight)
    {
        if (vertexIndex1 < 0 || (uint32_t)vertexIndex1 >= m_VertexSize || vertexIndex1 > vertexIndex2 ||
            vertexIndex2 < 0 || (uint32_t)vertexIndex2 >= m_VertexSize || boneIndex < 0 || weight < 0)
            return;

        for (int i = vertexIndex1; i < vertexIndex2 + 1; i++)
        {
            for (int j = 0; j < MAX_BONE_INFLUENCE; j++)
                if (m_Vertices[i].BoneIDs[j] == -1 || m_Vertices[i].BoneIDs[j] == boneIndex)
                {
                    m_Vertices[i].BoneIDs[j] = boneIndex;
                    m_Vertices[i].Weights[j] = weight;
                    break;
                }
        }

    }

    void Mesh::SetVertexBoneDataToDefault()
    {
        for(auto& vertex : m_Vertices)
            for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
            {
                vertex.BoneIDs[i] = -1;
                vertex.Weights[i] = 0.0f;
            }
    }


    void Mesh::SetVertexBoneDataToDefault(MeshVertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.BoneIDs[i] = -1;
            vertex.Weights[i] = 0.0f;
        }
    }

    void Mesh::SetVertexBoneData(MeshVertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.BoneIDs[i] < 0)
            {
                vertex.Weights[i] = weight;
                vertex.BoneIDs[i] = boneID;
                break;
            }
        }
    }

    void Mesh::SetupMesh()
    {
        m_VertexArray = VertexArray::Create();
        //vb = VertexBuffer::Create((void*)&vertices[0], vertices.size() * sizeof(MeshVertex));
        m_VertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(MeshVertex));
        m_VertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position"        },
            { ShaderDataType::Float2, "a_TexCoords"       },
            { ShaderDataType::Float3, "a_Normal"          },
            { ShaderDataType::Float3, "a_Tangent"         },
            { ShaderDataType::Float3, "a_Bitangent"       },
            { ShaderDataType::Int4,   "a_BoneIDs"         },
            { ShaderDataType::Float4, "a_Weights"         },
            { ShaderDataType::Int,    "a_EntityID"        }
            });
        m_VertexArray->AddVertexBuffer(m_VertexBuffer);
        vertexBufferBase = new MeshVertex[MaxVertices];

        uint32_t* indicesBuffer = new uint32_t[MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < MaxIndices; i += m_IndexSize)
        {
            for (uint32_t j = 0; j < m_IndexSize; j++)
                indicesBuffer[i + j] = offset + m_Indices[j];
            offset += m_VertexSize; // +vertexSize
        }
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indicesBuffer, MaxIndices);;
        m_VertexArray->SetIndexBuffer(indexBuffer);
        delete[] indicesBuffer;	// cpu上传到gpu上了可以删除cpu的索引数据块了

    }

    const Scope<MeshLibrary>& Mesh::GetMeshLibrary()
    {
        std::call_once(init_flag, []() { m_MeshLibrary.reset(new MeshLibrary()); });
        return m_MeshLibrary;
    }


    void MeshLibrary::Add(const Ref<MeshNode> meshNode)
    {
        if(meshNode->name != std::string())
            AddOrReplace(meshNode->name, meshNode);
        else
            VOL_ASSERT(false, "MeshLibrary::Add: Bug");
    }

    // path: mms文件的相对路径
    void MeshLibrary::AddOrReplace(const std::string& path, const Ref<MeshNode> meshNode)
    {
        VOL_CORE_ASSERT(!Exists(path), "ModelLibrary:Mesh已经存在了");
        m_Meshes[path] = meshNode;
    }

    // path: mms文件的相对路径
    Ref<MeshNode> MeshLibrary::Load(const std::string filepath)
    {
        Ref<MeshNode> meshNode = std::make_shared<MeshNode>();
        MeshSerializer serializer(meshNode);
        serializer.Deserialize(Project::GetAssetFileSystemPath(filepath).string());
        meshNode->name = filepath;
        Add(meshNode);
        return meshNode;
    }


    // path: mms文件的相对路径
    Ref<MeshNode> MeshLibrary::Get(const std::string& path)
    {
        if (!Exists(path))
        {
            VOL_CORE_TRACE("ModelLibrary:未找到Mesh");
            return nullptr;
        }
        return m_Meshes[path];
    }

    bool MeshLibrary::Exists(const std::string& path)
    {
        return m_Meshes.find(path) != m_Meshes.end();
    }
    void MeshLibrary::Remove(const std::string& path)
    {
        m_Meshes.erase(path);
    }
}