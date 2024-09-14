#include "volpch.h"
#include "Mesh.h"
#include "glad/glad.h"
#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	Mesh::Mesh(std::vector<MeshVertex> vertices, std::vector<uint32_t> indices, std::vector<MeshTexture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		setupMesh();
	}
    void Mesh::setupMesh()
    {
        va = VertexArray::Create();
        //vb = VertexBuffer::Create((void*)&vertices[0], vertices.size() * sizeof(MeshVertex));
        vb = VertexBuffer::Create(vertices.size() * sizeof(MeshVertex));
        vb->SetLayout({
            { ShaderDataType::Float3, "a_Position"        },
            { ShaderDataType::Float3, "a_Normal"          },
            { ShaderDataType::Float2, "a_TexCoords"       },
            { ShaderDataType::Float3, "a_Tangent"         },
            { ShaderDataType::Float3, "a_Bitangent"       },
            { ShaderDataType::Int,    "a_EntityID"        }
            });
        va->AddVertexBuffer(vb);
        Ref<IndexBuffer> ib = IndexBuffer::Create(&indices[0], indices.size());
        va->SetIndexBuffer(ib);
        va->UnBind();
        vertexBufferBase = new MeshVertex[vertices.size()];

        m_BlackTexture = Texture2D::Create(1, 1);
        uint32_t blackTextureData = 0x00000000;
        m_BlackTexture->SetData(&blackTextureData, sizeof(uint32_t));
    }

    void Mesh::Draw(Shader& shader, const glm::mat4& transform, const glm::mat3& normalTransform, int entityID)
    {
        /*
        uint32_t diffuseIndex = 1;
        uint32_t specularIndex = 1;
        uint32_t normalIndex = 1;
        uint32_t heightIndex = 1;
        const uint32_t vertexsSize = sizeof(vertices) / sizeof(uint32_t);
        */

        /*
        for (uint32_t i = 0; i < textures.size(); i++)
        {
            std::string number;
            std::string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseIndex++);
            else if (name == "texture_specular")
                number = std::to_string(specularIndex++);
            else if (name == "texture_normal")
                number = std::to_string(normalIndex++);
            else if (name == "texture_height")
                number = std::to_string(heightIndex++);

            textures[i].texture->Bind(i);
            // 设置每个着色器采样器属于哪个纹理单元。如texture_diffuse1使用第0个纹理单元
            shader.SetInt((name + number).c_str(), i);

        }
        */
        
        vertexBufferPtr = vertexBufferBase;
        for (uint32_t i = 0; i < vertices.size(); i++)
        {
            vertexBufferPtr->Position  = transform * glm::vec4(vertices[i].Position, 1.0f);
            //vertexBufferPtr->Position  = vertices[i].Position;
            vertexBufferPtr->Normal    = normalTransform * vertices[i].Normal;
            //vertexBufferPtr->Normal    = vertices[i].Normal;
            vertexBufferPtr->TexCoords = vertices[i].TexCoords;
            vertexBufferPtr->Tangent   = normalTransform * vertices[i].Tangent;
            vertexBufferPtr->Bitangent = glm::cross(vertexBufferPtr->Normal, vertexBufferPtr->Tangent);//normalTransform * vertices[i].Bitangent;
            vertexBufferPtr->EntityID  = entityID;
            vertexBufferPtr++;
        }
        vb->SetData(vertexBufferBase, vertices.size() * sizeof(MeshVertex));

    }

    void Mesh::DrawIndexed()
    {
        // TODO：Vulkan不能自动自由注入uniform，这里手动按顺序绑定纹理单元
        m_BlackTexture->Bind(0);
        m_BlackTexture->Bind(1);
        m_BlackTexture->Bind(2);
        for (uint32_t i = 0; i < textures.size(); i++)
        {
            std::string name = textures[i].type;
            if (name == "texture_diffuse")
                textures[i].texture->Bind(0);
            else if (name == "texture_specular")
                textures[i].texture->Bind(1);
            else if (name == "texture_normal")
                textures[i].texture->Bind(2);
        }

        Renderer::DrawIndexed(va, va->GetIndexBuffer()->GetCount());
    }

}