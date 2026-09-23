#include "volpch.h"
#include "Mesh.h"

#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/UniformBuffer.h"
#include "Volcano/Scene/Entity.h"
#include "Volcano/Scene/SceneSerializer/SceneSerializer.h"
#include "Volcano/Project/Project.h"
#include "Volcano/Renderer/RendererItem/Quad.h"
#include "Volcano/Renderer/RendererItem/Circle.h"
#include "Volcano/Renderer/RendererItem/Line.h"
#include "Volcano/Renderer/RendererItem/Plane.h"
#include "Volcano/Renderer/RendererItem/Cube.h"
#include "Volcano/Renderer/RendererItem/Sphere.h"
#include "Volcano/Renderer/RendererItem/Cylinder.h"
#include "Volcano/Renderer/RendererItem/Capsule.h"
#include "Volcano/Renderer/RendererItem/Cone.h"
#include "Volcano/Renderer/RendererItem/Skybox.h"

#include "Volcano/Renderer/InstanceData.h"


namespace Volcano {

    std::once_flag Mesh::m_MeshLibraryInitFlag;
    Scope<MeshLibrary> Mesh::m_MeshLibrary;

    static Ref<Texture2D> s_WhiteTexture;
    static Ref<Texture2D> s_BlackTexture;

    static Ref<TextureCube> s_WhiteTextureCube;
    static Ref<TextureCube> s_BlackTextureCube;

    void Mesh::Init()
    {
        uint64_t whiteHash = Texture::GetTextureLibrary()->m_WhiteHash;
        s_WhiteTexture = Texture::GetTextureLibrary()->Get("White", whiteHash, true);
        uint64_t blackHash = Texture::GetTextureLibrary()->m_BlackHash;
        s_BlackTexture = Texture::GetTextureLibrary()->Get("Black", blackHash, true);

        s_WhiteTextureCube = TextureCube::Create(512, 512, TextureInternalFormat::RGB16F, TextureDataFormat::RGB);
        s_WhiteTextureCube->SetData(glm::vec4(1.0f));
        s_BlackTextureCube = TextureCube::Create(512, 512, TextureInternalFormat::RGB16F, TextureDataFormat::RGB);
        s_BlackTextureCube->SetData(glm::vec4(0.0f));

    }
    
    const Scope<MeshLibrary>& Mesh::GetMeshLibrary()
    {
        std::call_once(m_MeshLibraryInitFlag, []() { m_MeshLibrary.reset(new MeshLibrary()); });
        return m_MeshLibrary;
    }


    Ref<Texture2D> Mesh::GetWhiteTexture()
    {
        return s_WhiteTexture;
    }

    Ref<Texture2D> Mesh::GetBlackTexture()
    {
        return s_BlackTexture;
    }

    Ref<TextureCube> Mesh::GetWhiteTextureCube()
    {
        return s_WhiteTextureCube;
    }

    Ref<TextureCube> Mesh::GetBlackTextureCube()
    {
        return s_BlackTextureCube;
    }

    void Mesh::BeginScene()
    {
        float Shininess = 32.0f;
        UniformBufferManager::GetUniformBuffer("Material")->SetData(&Shininess, sizeof(float));

        auto& meshLibrary = GetMeshLibrary();
        for (auto& [key, meshNode] : meshLibrary->GetMeshes())
        {
            MeshType meshType = MeshLibrary::StringToMeshType(key);
            switch (meshType)
            {
            case MeshType::Quad:
            {
                std::dynamic_pointer_cast<MeshNode<Quad>>(meshNode)->mesh->StartBatch();
                break;
            }
            case MeshType::Circle:
            {
                std::dynamic_pointer_cast<MeshNode<Circle>>(meshNode)->mesh->StartBatch();
                break;
            }
            case MeshType::Line:
            {
                std::dynamic_pointer_cast<MeshNode<Line>>(meshNode)->mesh->StartBatch();
                break;
            }
            case MeshType::Plane:
            {
                std::dynamic_pointer_cast<MeshNode<Plane>>(meshNode)->mesh->StartBatch();
                break;
            }
            case MeshType::Cube:
            {
                std::dynamic_pointer_cast<MeshNode<Cube>>(meshNode)->mesh->StartBatch();
                break;
            }
            case MeshType::Sphere:
            {
                std::dynamic_pointer_cast<MeshNode<Sphere>>(meshNode)->mesh->StartBatch();
                break;
            }
            case MeshType::Cylinder:
            {
                std::dynamic_pointer_cast<MeshNode<Cylinder>>(meshNode)->mesh->StartBatch();
                break;
            }
            case MeshType::Capsule:
            {
                std::dynamic_pointer_cast<MeshNode<Capsule>>(meshNode)->mesh->StartBatch();
                break;
            }
            case MeshType::Cone:
            {
                std::dynamic_pointer_cast<MeshNode<Cone>>(meshNode)->mesh->StartBatch();
                break;
            }
            case MeshType::Skybox:
            {
                std::dynamic_pointer_cast<MeshNode<Skybox>>(meshNode)->mesh->StartBatch();
                break;
            }
            default:
                break;
            }
        }
    }

    void Mesh::EndScene()
    {
        auto& meshLibrary = GetMeshLibrary();
        for (auto& [key, meshNode] : meshLibrary->GetMeshes())
        {
            MeshType meshType = MeshLibrary::StringToMeshType(key);
            switch (meshType)
            {
            case MeshType::Quad:
            {
                std::dynamic_pointer_cast<MeshNode<Quad>>(meshNode)->mesh->Flush();
                break;
            }
            case MeshType::Circle:
            {
                std::dynamic_pointer_cast<MeshNode<Circle>>(meshNode)->mesh->Flush();
                break;
            }
            case MeshType::Line:
            {
                std::dynamic_pointer_cast<MeshNode<Line>>(meshNode)->mesh->Flush();
                break;
            }
            case MeshType::Plane:
            {
                std::dynamic_pointer_cast<MeshNode<Plane>>(meshNode)->mesh->Flush();
                break;
            }
            case MeshType::Cube:
            {
                std::dynamic_pointer_cast<MeshNode<Cube>>(meshNode)->mesh->Flush();
                break;
            }
            case MeshType::Sphere:
            {
                std::dynamic_pointer_cast<MeshNode<Sphere>>(meshNode)->mesh->Flush();
                break;
            }
            case MeshType::Cylinder:
            {
                std::dynamic_pointer_cast<MeshNode<Cylinder>>(meshNode)->mesh->Flush();
                break;
            }
            case MeshType::Capsule:
            {
                std::dynamic_pointer_cast<MeshNode<Capsule>>(meshNode)->mesh->Flush();
                break;
            }
            case MeshType::Cone:
            {
                std::dynamic_pointer_cast<MeshNode<Cone>>(meshNode)->mesh->Flush();
                break;
            }
            case MeshType::Skybox:
            {
                std::dynamic_pointer_cast<MeshNode<Skybox>>(meshNode)->mesh->Flush();
                break;
            }
            default:
                break;
            }
        }
    }

    Mesh::Mesh()
    {
        m_BufferLayout = {
            { ShaderDataType::Float3, "a_Position"     },
            { ShaderDataType::Float2, "a_TexCoord"     },
            { ShaderDataType::Float3, "a_Normal"       },
            { ShaderDataType::Float3, "a_Tangent"      },
            { ShaderDataType::Float3, "a_Bitangent"    }
        };
    }

    void Mesh::StartBatch()
    {
        ClearInstance();
    }

    void Mesh::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Mesh::Flush()
    {
        auto instanceCount = m_InstanceDataList.size();
        if (instanceCount != 0)
        {
            UniformBufferManager::GetUniformBuffer("InstanceData")->SetData(m_InstanceDataList.data(), instanceCount * sizeof(InstanceData));
            UniformBufferManager::GetUniformBuffer("InstanceDataMaterial")->SetData(m_InstanceDataMaterialList.data(), instanceCount * sizeof(InstanceDataMaterial));
            UniformBufferManager::GetUniformBuffer("InstanceDataExplosion")->SetData(m_InstanceDataExplosionList.data(), instanceCount * sizeof(InstanceDataExplosion));
            UniformBufferManager::GetUniformBuffer("InstanceDataOutline")->SetData(m_InstanceDataOutlineList.data(), instanceCount * sizeof(InstanceDataOutline));
            UniformBufferManager::GetUniformBuffer("InstanceDataNormalVisualization")->SetData(m_InstanceDataNormalVisualizationList.data(), instanceCount * sizeof(InstanceDataNormalVisualization));
            UniformBufferManager::GetUniformBuffer("InstanceDataEntityID")->SetData(m_InstanceDataEntityIDList.data(), instanceCount * sizeof(InstanceDataEntityID));
            UniformBufferManager::GetUniformBuffer("InstanceDataLightingMode")->SetData(m_InstanceDataLightingModeList.data(), instanceCount * sizeof(InstanceDataLightingMode));
            FlushInstances();
        }
    }

    void Mesh::ClearInstance()
    {
        m_InstanceDataList.clear();
        m_InstanceDataMaterialList.clear();
        m_InstanceDataExplosionList.clear();
        m_InstanceDataOutlineList.clear();
        m_InstanceDataNormalVisualizationList.clear();
        m_InstanceDataEntityIDList.clear();
        m_InstanceDataLightingModeList.clear();
    }

    void Mesh::AddInstance(InstanceDataTotal instanceDataTotal)
    {
        m_InstanceDataList.push_back(std::move(instanceDataTotal.instanceData));
        m_InstanceDataMaterialList.push_back(std::move(instanceDataTotal.material));
        m_InstanceDataExplosionList.push_back(std::move(instanceDataTotal.explosion));
        m_InstanceDataOutlineList.push_back(std::move(instanceDataTotal.outline));
        m_InstanceDataNormalVisualizationList.push_back(std::move(instanceDataTotal.normalVisualization));
        m_InstanceDataEntityIDList.push_back(std::move(instanceDataTotal.entityID));
        m_InstanceDataLightingModeList.push_back(std::move(instanceDataTotal.lightingMode));
    }

    // =================================================================================================
    // MeshLibrary
    // =================================================================================================

    MeshType MeshLibrary::StringToMeshType(const std::string& str)
    {
        static const std::unordered_map<std::string, MeshType> typeMap = {
            {"class Volcano::Quad",     MeshType::Quad     },
            {"class Volcano::Circle",   MeshType::Circle   },
            {"class Volcano::Line",     MeshType::Line     },
            {"class Volcano::Plane",    MeshType::Plane    },
            {"class Volcano::Cube",     MeshType::Cube     },
            {"class Volcano::Sphere",   MeshType::Sphere   },
            {"class Volcano::Cylinder", MeshType::Cylinder },
            {"class Volcano::Capsule",  MeshType::Capsule  },
            {"class Volcano::Cone",     MeshType::Cone     },
            {"class Volcano::Model",    MeshType::Model    },
            {"class Volcano::Skybox",   MeshType::Skybox   }
        };

        auto it = typeMap.find(str);
        if (it != typeMap.end())
            return it->second;

        return MeshType::None;
    }

    std::string MeshLibrary::MeshTypeToString(MeshType meshType)
    {
        static const std::unordered_map<MeshType, std::string> typeMap = {
            { MeshType::Quad,    "class Volcano::Quad"     },
            { MeshType::Circle,  "class Volcano::Circle"   },
            { MeshType::Line,    "class Volcano::Line"     },
            { MeshType::Plane,   "class Volcano::Plane"    },
            { MeshType::Cube,    "class Volcano::Cube"     },
            { MeshType::Sphere,  "class Volcano::Sphere"   },
            { MeshType::Cylinder,"class Volcano::Cylinder" },
            { MeshType::Capsule, "class Volcano::Capsule"  },
            { MeshType::Cone,    "class Volcano::Cone"     },
            { MeshType::Model,   "class Volcano::Model"    },
            { MeshType::Skybox,  "class Volcano::Skybox"   }
        };

        auto it = typeMap.find(meshType);
        if (it != typeMap.end())
            return it->second;

        return std::string();
    }

    template<typename TMesh>
    void MeshLibrary::Add(const Ref<MeshNode<TMesh>>& meshNode)
    {
        static_assert(std::is_base_of_v<Mesh, TMesh>, "MeshLibrary::Add: TMesh must be derived from Mesh");
        VOL_CORE_ASSERT(meshNode != nullptr && meshNode->mesh != nullptr, "MeshLibrary::Add: meshNode should not be a null pointer");

        AddWithName<TMesh>(typeid(TMesh).name(), meshNode);
    }

    template<typename TMesh>
    void MeshLibrary::AddWithName(const std::string& key, const Ref<MeshNode<TMesh>>& meshNode)
    {
        VOL_CORE_ASSERT(!Exists(key), "MeshLibrary:Mesh已经存在了");
        // typeid(TMesh) 是 C++ 运行时类型识别（RTTI，Run-Time Type Identification）的一部分，
        // 它返回一个 std::type_info 对象的引用，用于表示类型 TMesh 的唯一信息
        m_Meshes[key] = std::static_pointer_cast<MeshNodeBase>(meshNode);;
    }

    // path: mms文件的相对路径
    template<typename TMesh>
    Ref<MeshNode<TMesh>> MeshLibrary::Load(const std::string key)
    {
        Ref<MeshNode<TMesh>> meshNode = std::make_shared<MeshNode<TMesh>>();
        return meshNode;
    }



    void MeshLibrary::Add(const std::string& key)
    {
        MeshType meshType = StringToMeshType(key);
        switch (meshType)
        {
        case MeshType::Quad:
        {
            auto meshNode = CreateRef<MeshNode<Quad>>();
            meshNode->mesh = CreateRef<Quad>();
            Add<Quad>(meshNode);
            break;
        }
        case MeshType::Circle:
        {
            auto meshNode = CreateRef<MeshNode<Circle>>();
            meshNode->mesh = CreateRef<Circle>();
            Add<Circle>(meshNode);
            break;
        }
        case MeshType::Line:
        {
            auto meshNode = CreateRef<MeshNode<Line>>();
            meshNode->mesh = CreateRef<Line>();
            Add<Line>(meshNode);
            break;
        }
        case MeshType::Plane:
        {
            auto meshNode = CreateRef<MeshNode<Plane>>();
            meshNode->mesh = CreateRef<Plane>();
            Add<Plane>(meshNode);
            break;
        }
        case MeshType::Cube:
        {
            auto meshNode = CreateRef<MeshNode<Cube>>();
            meshNode->mesh = CreateRef<Cube>();
            Add<Cube>(meshNode);
            break;
        }
        case MeshType::Sphere:
        {
            auto meshNode = CreateRef<MeshNode<Sphere>>();
            meshNode->mesh = CreateRef<Sphere>();
            Add<Sphere>(meshNode);
            break;
        }
        case MeshType::Cylinder:
        {
            auto meshNode = CreateRef<MeshNode<Cylinder>>();
            meshNode->mesh = CreateRef<Cylinder>();
            Add<Cylinder>(meshNode);
            break;
        }
        case MeshType::Capsule:
        {
            auto meshNode = CreateRef<MeshNode<Capsule>>();
            meshNode->mesh = CreateRef<Capsule>();
            Add<Capsule>(meshNode);
            break;
        }
        case MeshType::Cone:
        {
            auto meshNode = CreateRef<MeshNode<Cone>>();
            meshNode->mesh = CreateRef<Cone>();
            Add<Cone>(meshNode);
            break;
        }
        case MeshType::Skybox:
        {
            auto meshNode = CreateRef<MeshNode<Skybox>>();
            meshNode->mesh = CreateRef<Skybox>();
            Add<Skybox>(meshNode);
            break;
        }
        default:
            break;
        }
    }

    bool MeshLibrary::Exists(const std::string& key)
    {
        return m_Meshes.find(key) != m_Meshes.end();
    }
    void MeshLibrary::Remove(const std::string& key)
    {
        m_Meshes.erase(key);
    }

}