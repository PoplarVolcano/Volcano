#include "ExampleLayer.h"

#include <glm/ext/matrix_transform.hpp>
#include "Volcano/Renderer/Renderer2D.h"
#include "Volcano/Renderer/RendererItem/Skybox.h"
#include "Volcano/Renderer/RendererItem/Sphere.h"

#include <chrono>
#include <imgui/imgui.h>
#include <Volcano/ImGui/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

#include "Volcano/Scene/SceneSerializer.h"
#include "Volcano/Utils/PlatformUtils.h"
#include "Volcano/Math/Math.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Core/MouseBuffer.h"
#include "Volcano/Core/Window.h"
#include <random>


namespace Volcano{

    static Ref<UniformBuffer> s_BlurUniformBuffer;
    static Ref<UniformBuffer> s_ExposureUniformBuffer;
    static float s_Exposure = 1.5f;
    static bool s_Bloom = true;
    static Ref<UniformBuffer> s_SamplesUniformBuffer;
    static Ref<UniformBuffer> s_SSAOUniformBuffer;
    static int s_KernelSize = 64;
    static float s_Radius = 0.5;
    static float s_Bias = 0.035;
    static float s_Power = 1.0;
    static Ref<UniformBuffer> s_PBRUniformBuffer;
    static Ref<UniformBuffer> s_PrefilterUniformBuffer;

    template<typename Fn>
    class Timer {
    public:
        Timer(const char* name, Fn&& func)
            :m_Name(name), m_Func(func), m_Stopped(false)
        {
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }
        ~Timer() {
            if (!m_Stopped) {
                Stop();
            }
        }
        void Stop() {
            //���ٶ���ʱ�������ʱ����δֹͣ �������¼����ʱ���
            auto endTimepoint = std::chrono::high_resolution_clock::now();
            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
            m_Stopped = true;
            float duration = (end - start) * 0.001f;// ��λms
            m_Func({ m_Name, duration });
            //std::cout << "Timer:"<< m_Name << "ʱ�" << duration << "ms" << std::endl;
        }
    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
        bool m_Stopped;
        Fn m_Func;
    };

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); })

    ExampleLayer::ExampleLayer()
    {
    }

    ExampleLayer::~ExampleLayer()
    {
    }

    void ExampleLayer::OnAttach()
    {
        m_IconPlay = Texture2D::Create("Resources/Icons/PlayButton.png");
        m_IconPause = Texture2D::Create("Resources/Icons/PauseButton.png");
        m_IconStop = Texture2D::Create("Resources/Icons/StopButton.png");
        m_IconSimulate = Texture2D::Create("Resources/Icons/SimulateButton.png");
        m_IconStep = Texture2D::Create("Resources/Icons/StepButton.png");

        // ��FrameBuffer��ͼ��ŵ�window��

        m_WindowVertex[0].Position = { -1.0, -1.0, 0.0f };
        m_WindowVertex[1].Position = { 1.0, -1.0, 0.0f };
        m_WindowVertex[2].Position = { 1.0,  1.0, 0.0f };
        m_WindowVertex[3].Position = { -1.0,  1.0, 0.0f };
        m_WindowVertex[0].TextureCoords = { 0.0f, 0.0f };
        m_WindowVertex[1].TextureCoords = { 1.0f, 0.0f };
        m_WindowVertex[2].TextureCoords = { 1.0f, 1.0f };
        m_WindowVertex[3].TextureCoords = { 0.0f, 1.0f };

        windowVa = VertexArray::Create();
        Ref<VertexBuffer> windowVb = VertexBuffer::Create((void*)&m_WindowVertex[0], sizeof(m_WindowVertex));
        windowVb->SetLayout({
            { ShaderDataType::Float3, "a_Position"     },
            { ShaderDataType::Float2, "a_TexCoords"    }
            });
        windowVa->AddVertexBuffer(windowVb);
        uint32_t indices[] = { 0, 1, 2, 2, 3 ,0 };
        Ref<IndexBuffer> windowIb = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));;
        windowVa->SetIndexBuffer(windowIb);
        m_WindowShader = Renderer::GetShaderLibrary()->Get("window");
        m_HDRShader = Renderer::GetShaderLibrary()->Get("HDR");


        // =========================================Framebuffer===============================================
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = {
            FramebufferTextureFormat::RGBA16F,
            FramebufferTextureFormat::RED_INTEGER,
            FramebufferTextureFormat::RGBA16F,
            FramebufferTextureFormat::Depth     //���һ����ȸ�����Ĭ��Depth = DEPTH24STENCIL8
        };
        fbSpec.Width  = 1280;
        fbSpec.Height = 720;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        fbSpec.Attachments = {
            FramebufferTextureFormat::DEPTH_COMPONENT
        };
        fbSpec.Width  = 1024;
        fbSpec.Height = 1024;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_DirectionalDepthMapFramebuffer = Framebuffer::Create(fbSpec);

        
        fbSpec.Attachments = {
            FramebufferTextureFormat::DEPTH_COMPONENT
        };
        fbSpec.Width = 1024;
        fbSpec.Height = 1024;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_CUBE_MAP;
        m_PointDepthMapFramebuffer = Framebuffer::Create(fbSpec);

        fbSpec.Attachments = {
            FramebufferTextureFormat::DEPTH_COMPONENT
        };
        fbSpec.Width = 1024;
        fbSpec.Height = 1024;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_SpotDepthMapFramebuffer = Framebuffer::Create(fbSpec);
        

        fbSpec.Attachments = {
            FramebufferTextureFormat::RGBA16F
        };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_BlurFramebuffer[0] = Framebuffer::Create(fbSpec);
        m_BlurFramebuffer[1] = Framebuffer::Create(fbSpec);
        m_HDRFramebuffer     = Framebuffer::Create(fbSpec);

        s_ExposureUniformBuffer = UniformBuffer::Create(2 * sizeof(float), 11);
        s_BlurUniformBuffer = UniformBuffer::Create(sizeof(float), 12);

        s_ExposureUniformBuffer->SetData(&s_Exposure, sizeof(float));
        s_ExposureUniformBuffer->SetData(&s_Bloom, sizeof(bool), sizeof(float));


        fbSpec.Attachments = {
            FramebufferTextureFormat::RGBA16F,  // λ��+��Ȼ���
            FramebufferTextureFormat::RGBA16F,  // ����+EntityID����
            FramebufferTextureFormat::RGBA8,    // ��ɫ����
            FramebufferTextureFormat::RGBA8,    // ���滺��
            FramebufferTextureFormat::Depth
        };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_GBufferFramebuffer = Framebuffer::Create(fbSpec);

        fbSpec.Attachments = {
            FramebufferTextureFormat::RGBA16F,
            FramebufferTextureFormat::RED_INTEGER,
            FramebufferTextureFormat::RGBA16F,
            FramebufferTextureFormat::Depth     //���һ����ȸ�����Ĭ��Depth = DEPTH24STENCIL8
        };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_DeferredShadingFramebuffer = Framebuffer::Create(fbSpec);

        fbSpec.Attachments = {
            FramebufferTextureFormat::RED
        };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_SSAOFramebuffer = Framebuffer::Create(fbSpec);

        fbSpec.Attachments = {
            FramebufferTextureFormat::RED
        };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_SSAOBlurFramebuffer = Framebuffer::Create(fbSpec);

        // Sample kernel
        // ��������
        std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // �����������[0.0, 1.0]
        std::default_random_engine generator;
        std::vector<glm::vec3> ssaoKernel;
        for (uint32_t i = 0; i < 64; ++i)
        {
            glm::vec3 sample(
                randomFloats(generator) * 2.0 - 1.0, 
                randomFloats(generator) * 2.0 - 1.0, 
                randomFloats(generator));
            sample = glm::normalize(sample);
            sample *= randomFloats(generator);
            float scale = float(i) / 64.0;

            // ��������������ԭ��ֲ�Scale samples s.t. they're more aligned to center of kernel
            scale = 0.1f + scale * scale * (1.0f - 0.1f);//lerp(0.1f, 1.0f, scale * scale);  //return a + f * (b - a);
            sample *= scale;
            ssaoKernel.push_back(sample);
        }
        s_SamplesUniformBuffer = UniformBuffer::Create(ssaoKernel.size() * 4 * sizeof(float), 13);
        for (uint32_t i = 0; i < ssaoKernel.size(); i++)
            s_SamplesUniformBuffer->SetData(&ssaoKernel[i], sizeof(glm::vec3), i * 4 * sizeof(float));


        // Noise texture
        // 4x4�������߿ռ�ƽ�淨�ߵ������ת��������
        std::vector<glm::vec3> ssaoNoise;
        for (uint32_t i = 0; i < 16; i++)
        {
            glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
            ssaoNoise.push_back(noise);
        }

        m_NoiseTexture = Texture2D::Create(4, 4, TextureFormat::RGBA16F, TextureFormat::RGB);
        m_NoiseTexture->SetData(&ssaoNoise[0], ssaoNoise.size() * 3);

        s_SSAOUniformBuffer = UniformBuffer::Create(4 * sizeof(float), 14);



        fbSpec.Attachments = {
            FramebufferTextureFormat::RGBA16F
        };
        fbSpec.Width = 512;
        fbSpec.Height = 512;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_PBRFramebuffer = Framebuffer::Create(fbSpec);

        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViewProjections[] =
        {
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };
        for(uint32_t i = 0; i < 6; i++)
            captureViewProjections[i] = captureProjection * captureViewProjections[i];

        s_PBRUniformBuffer = UniformBuffer::Create(4 * 4 * sizeof(float), 16);
        m_EquirectangularMap = Texture2D::Create("SandBoxProject/Assets/Textures/hdr/newport_loft.hdr", true, TextureFormat::RGB16F);
        m_EnvCubemap = TextureCube::Create(TextureFormat::RGB16F, 512, 512);
        
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        va = VertexArray::Create();
        Ref<VertexBuffer> vb = VertexBuffer::Create(vertices, sizeof(vertices));
        vb->SetLayout({
            { ShaderDataType::Float3, "a_Position"     },
            { ShaderDataType::Float3, "a_Normal"       },
            { ShaderDataType::Float2, "a_TexCoords"    }
            });
        va->AddVertexBuffer(vb);
        uint32_t cubeIndicesBuffer[36];
        for (uint32_t i = 0; i < 36; i++)
            cubeIndicesBuffer[i] = i;
        Ref<IndexBuffer> ib = IndexBuffer::Create(cubeIndicesBuffer, 36);
        va->SetIndexBuffer(ib);
        Renderer::SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        m_PBRFramebuffer->Bind();
        {
            Renderer::GetShaderLibrary()->Get("EquirectangularToCubemap")->Bind();
            m_EquirectangularMap->Bind();
            
            for (uint32_t i = 0; i < 6; ++i)
            {
                s_PBRUniformBuffer->SetData(&captureViewProjections[i], 4 * 4 * sizeof(float));
                // ��envCubemap��6�������ΰ󶨵�m_PBRFramebuffer����ɫ����
                m_PBRFramebuffer->SetColorAttachment(m_EnvCubemap, TextureType(uint32_t(TextureType::TEXTURE_CUBE_MAP_POSITIVE_X) + i));
                Renderer::Clear();

                // ȡ�����޳�
                RendererAPI::SetCullFace(false);
                Renderer::DrawIndexed(va, va->GetIndexBuffer()->GetCount());
                RendererAPI::SetCullFace(true);

            }
            m_PBRFramebuffer->Unbind();
        }

        std::vector<std::string> faces
        {
            std::string("SandBoxProject/Assets/Textures/skybox/right.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/left.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/top.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/bottom.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/front.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/back.jpg"),
        };
        m_EnvCubemap = TextureCube::Create(faces);

        m_IrradianceMap = TextureCube::Create(TextureFormat::RGB16F, 32, 32);
        m_PBRFramebuffer->Resize(32, 32);

        m_PBRFramebuffer->Bind();
        {
            Renderer::GetShaderLibrary()->Get("IrradianceConvolution")->Bind();
            m_EnvCubemap->Bind();

            for (uint32_t i = 0; i < 6; ++i)
            {
                s_PBRUniformBuffer->SetData(&captureViewProjections[i], 4 * 4 * sizeof(float));
                m_PBRFramebuffer->SetColorAttachment(m_IrradianceMap, TextureType(uint32_t(TextureType::TEXTURE_CUBE_MAP_POSITIVE_X) + i));
                Renderer::Clear();
                RendererAPI::SetCullFace(false);
                Renderer::DrawIndexed(va, va->GetIndexBuffer()->GetCount());
                RendererAPI::SetCullFace(true);
            }
            m_PBRFramebuffer->Unbind();
        }

        s_PrefilterUniformBuffer = UniformBuffer::Create(sizeof(float), 17);
        uint32_t prefilterMapWidth = 128;
        uint32_t prefilterMapHeight = 128;
        m_PrefilterMap = TextureCube::Create(TextureFormat::RGB16F, prefilterMapWidth, prefilterMapHeight);

        uint32_t maxMipLevels = 5;
        for (uint32_t mip = 0; mip < maxMipLevels; ++mip)
        {
            // reisze framebuffer according to mip-level size.
            prefilterMapWidth  = static_cast<uint32_t>(128 * std::pow(0.5, mip));
            prefilterMapHeight = static_cast<uint32_t>(128 * std::pow(0.5, mip));
            m_PBRFramebuffer->Resize(prefilterMapWidth, prefilterMapHeight);
            m_PBRFramebuffer->Bind();
            {
                Renderer::GetShaderLibrary()->Get("Prefilter")->Bind();
                m_EnvCubemap->Bind();
                float roughness = (float)mip / (float)(maxMipLevels - 1);
                s_PrefilterUniformBuffer->SetData(&roughness, sizeof(float));
                for (uint32_t i = 0; i < 6; ++i)
                {
                    s_PBRUniformBuffer->SetData(&captureViewProjections[i], 4 * 4 * sizeof(float));
                    m_PBRFramebuffer->SetColorAttachment(m_PrefilterMap, TextureType(uint32_t(TextureType::TEXTURE_CUBE_MAP_POSITIVE_X) + i), 0, mip);
                    Renderer::Clear();
                    RendererAPI::SetCullFace(false);
                    Renderer::DrawIndexed(va, va->GetIndexBuffer()->GetCount());
                    RendererAPI::SetCullFace(true);
                }
                m_PBRFramebuffer->Unbind();
            }
        }

        /*
        m_BRDFLUT = Texture2D::Create(512, 512, TextureFormat::RG16F, TextureFormat::RG, TextureWrap::CLAMP_TO_EDGE);
        m_PBRFramebuffer->Resize(512, 512);
        m_PBRFramebuffer->Bind();
        {
            Renderer::GetShaderLibrary()->Get("BRDF")->Bind();
            m_PBRFramebuffer->SetColorAttachment(m_BRDFLUT, TextureType::TEXTURE_2D);
            Renderer::Clear();
            Renderer::DrawIndexed(windowVa, windowVa->GetIndexBuffer()->GetCount());
            m_PBRFramebuffer->Unbind();
        }
        */
        m_BRDFLUT = Texture2D::Create("SandBoxProject/Assets/Textures/BRDF_LUT.tga", true, TextureFormat::RG16F);
        
        Sphere::SetIrradianceMap(m_IrradianceMap);
        Sphere::SetPrefilterMap(m_PrefilterMap);
        Sphere::SetBRDFLUT(m_BRDFLUT);
        Skybox::SetTexture(m_EnvCubemap);
        

        //================================================================================================

        // ��ʼ������
        m_EditorScene = CreateRef<Scene>();
        m_ActiveScene = m_EditorScene;

        // ��app��ȡָ���У����ָ���д���1���ȡ����
        auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
        if (commandLineArgs.Count > 1)
        {
            auto projectFilePath = commandLineArgs[1];
            OpenProject(projectFilePath);
        }
        else {
            // NOTE: this is while we don't have a new project path
            // �����û�ѡ��һ����Ŀ·��prompt the user to select a directory
            // ���û�д���Ŀ��ر�VolcanoNut If no project is opened, close VolcanoNut
            if (!OpenProject())
                Application::Get().Close();
        }

        m_EditorCamera = EditorCamera(30.0f, 1.788f, 0.1f, 1000.0f);

        Renderer2D::SetLineWidth(4.0f);

        
    }

    void ExampleLayer::OnDetach()
    {
    }

    void ExampleLayer::RenderScene(Timestep ts)
    {
        switch (m_SceneState)
        {
        case SceneState::Edit:
        {
            m_ActiveScene->OnRenderEditor(ts, m_EditorCamera);
            break;
        }
        case SceneState::Simulate:
        {
            m_ActiveScene->OnRenderSimulation(ts, m_EditorCamera);
            break;
        }
        case SceneState::Play:
        {
            m_ActiveScene->OnRenderRuntime(ts);
            break;
        }
        }
    }

    void ExampleLayer::OnUpdate(Timestep ts)
    {
        PROFILE_SCOPE("ExampleLayer::OnUpdate");

        m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        if (m_SceneState == SceneState::Play)
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

        //Resize
        FramebufferSpecification spec = m_Framebuffer->GetSpecification();
        if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized Framebuffer is invalid
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            // ����ͼ�ĳߴ�ͬ����֡����ߴ�
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // ������Ⱦ����
        Renderer2D::ResetStats();

        // ��������
        switch (m_SceneState)
        {
            case SceneState::Edit:
            {
                m_EditorCamera.OnUpdate(ts);
                m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
                break;
            }
            case SceneState::Simulate:
            {
                m_EditorCamera.OnUpdate(ts);
                m_ActiveScene->OnUpdateSimulation(ts, m_EditorCamera);
                break;
            }
            case SceneState::Play:
            {
                m_ActiveScene->OnUpdateRuntime(ts);
                break;
            }
        }

        Renderer::SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        //============================================Framebuffer=============================================
        // 
        // ��Ⱦ��Ӱ
        m_DirectionalDepthMapFramebuffer->Bind();
        {
            Renderer::Clear();

            //RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);

            m_ActiveScene->SetRenderType(RenderType::SHADOW_DIRECTIONALLIGHT);
            RenderScene(ts);
            m_ActiveScene->SetRenderType(RenderType::NORMAL);
            //RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

            m_DirectionalDepthMapFramebuffer->Unbind();
            // �������ͼ�󶨵�31������ۣ�����shader��ȡ��Ӱ����ȡ31�Ų�
            Texture::Bind(m_DirectionalDepthMapFramebuffer->GetDepthAttachmentRendererID(), 30);
        }


        m_PointDepthMapFramebuffer->Bind();
        {
            Renderer::Clear();
            //RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);
            m_ActiveScene->SetRenderType(RenderType::SHADOW_POINTLIGHT);
            RenderScene(ts);
            m_ActiveScene->SetRenderType(RenderType::NORMAL);
            //RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);
            m_PointDepthMapFramebuffer->Unbind();
            Texture::Bind(m_PointDepthMapFramebuffer->GetDepthAttachmentRendererID(), 31);
        }

        m_SpotDepthMapFramebuffer->Bind();
        {
            Renderer::Clear();

            //RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);

            m_ActiveScene->SetRenderType(RenderType::SHADOW_SPOTLIGHT);
            RenderScene(ts);
            m_ActiveScene->SetRenderType(RenderType::NORMAL);
            //RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

            m_SpotDepthMapFramebuffer->Unbind();
            Texture::Bind(m_SpotDepthMapFramebuffer->GetDepthAttachmentRendererID(), 29);
        }

        m_GBufferFramebuffer->Bind();
        {
            Renderer::Clear();
            m_GBufferFramebuffer->ClearAttachment(1, -1);

            m_ActiveScene->SetRenderType(RenderType::G_BUFFER);
            RenderScene(ts);
            m_ActiveScene->SetRenderType(RenderType::NORMAL);

            m_GBufferFramebuffer->Unbind();

        }
        /*
        m_DeferredShadingFramebuffer->Bind();
        {
            Renderer::Clear();

            // Clear entity ID attachment to -1
            m_DeferredShadingFramebuffer->ClearAttachment(1, -1);

            m_ActiveScene->SetRenderType(RenderType::DEFERRED_SHADING);

            Renderer::GetShaderLibrary()->Get("DeferredShading")->Bind();
            uint32_t positionTextureID   = m_GBufferFramebuffer->GetColorAttachmentRendererID(0);
            uint32_t normalTextureID     = m_GBufferFramebuffer->GetColorAttachmentRendererID(1);
            uint32_t albedoSpecTextureID = m_GBufferFramebuffer->GetColorAttachmentRendererID(2);
            uint32_t entityIDTextureID   = m_GBufferFramebuffer->GetColorAttachmentRendererID(3);
            Texture::Bind(positionTextureID,   0);
            Texture::Bind(normalTextureID,     1);
            Texture::Bind(albedoSpecTextureID, 2);
            Texture::Bind(entityIDTextureID,   3);
            Renderer::SetDepthTest(false);
            Renderer::DrawIndexed(windowVa, windowVa->GetIndexBuffer()->GetCount());
            Renderer::SetDepthTest(true);


            m_ActiveScene->SetRenderType(RenderType::NORMAL);

            m_DeferredShadingFramebuffer->Unbind();

        }
        */

        
        s_SSAOUniformBuffer->SetData(&s_KernelSize, sizeof(float));
        s_SSAOUniformBuffer->SetData(&s_Radius,     sizeof(float), sizeof(float));
        s_SSAOUniformBuffer->SetData(&s_Bias,       sizeof(float), 2 * sizeof(float));
        s_SSAOUniformBuffer->SetData(&s_Power,      sizeof(float), 3 * sizeof(float));

        m_SSAOFramebuffer->Bind();
        {
            Renderer::Clear();
            Renderer::GetShaderLibrary()->Get("SSAO")->Bind();
            uint32_t positionTextureID   = m_GBufferFramebuffer->GetColorAttachmentRendererID(0);
            uint32_t normalTextureID     = m_GBufferFramebuffer->GetColorAttachmentRendererID(1);
            Texture::Bind(positionTextureID,   0);
            Texture::Bind(normalTextureID,     1);
            m_NoiseTexture->Bind(2);
            Renderer::DrawIndexed(windowVa, windowVa->GetIndexBuffer()->GetCount());

            m_SSAOFramebuffer->Unbind();
        }
        
        m_SSAOBlurFramebuffer->Bind();
        {
            Renderer::Clear();
            Renderer::GetShaderLibrary()->Get("SSAOBlur")->Bind();
            uint32_t ssaoColorBuffer = m_SSAOFramebuffer->GetColorAttachmentRendererID(0);
            Texture::Bind(ssaoColorBuffer, 0);
            Renderer::DrawIndexed(windowVa, windowVa->GetIndexBuffer()->GetCount());
            m_SSAOFramebuffer->Unbind();
        }
        
        m_Framebuffer->BlitDepthFramebuffer(
            m_GBufferFramebuffer->GetRendererID(), m_Framebuffer->GetRendererID(),
            0, 0, m_Framebuffer->GetSpecification().Width, m_Framebuffer->GetSpecification().Height,
            0, 0, m_Framebuffer->GetSpecification().Width, m_Framebuffer->GetSpecification().Height);

        m_Framebuffer->Bind();
        {
            Renderer::Clear();

            // Clear entity ID attachment to -1
            m_Framebuffer->ClearAttachment(1, -1);
            
            m_ActiveScene->SetRenderType(RenderType::DEFERRED_SHADING);

            Renderer::GetShaderLibrary()->Get("DeferredShading")->Bind();
            uint32_t positionTextureID   = m_GBufferFramebuffer->GetColorAttachmentRendererID(0);
            uint32_t normalTextureID     = m_GBufferFramebuffer->GetColorAttachmentRendererID(1);
            uint32_t diffuseTextureID    = m_GBufferFramebuffer->GetColorAttachmentRendererID(2);
            uint32_t specularTextureID   = m_GBufferFramebuffer->GetColorAttachmentRendererID(3);
            uint32_t ssaoColorBufferBlur = m_SSAOBlurFramebuffer->GetColorAttachmentRendererID(0);
            Texture::Bind(positionTextureID,   0);
            Texture::Bind(normalTextureID,     1);
            Texture::Bind(diffuseTextureID,    2);
            Texture::Bind(specularTextureID,   3);
            Texture::Bind(ssaoColorBufferBlur, 4);
            //Renderer::SetDepthTest(false);
            Renderer::DrawIndexed(windowVa, windowVa->GetIndexBuffer()->GetCount());
            //Renderer::SetDepthTest(true);


            m_ActiveScene->SetRenderType(RenderType::NORMAL);
            RenderScene(ts);
            m_ActiveScene->SetRenderType(RenderType::SKYBOX);
            RenderScene(ts);
            m_ActiveScene->SetRenderType(RenderType::NORMAL);


            auto [mx, my] = ImGui::GetMousePos();
            // ������λ�ü�ȥviewport���ڵ����ϽǾ���λ��=��������viewport�������Ͻǵ�λ��
            mx -= m_ViewportBounds[0].x;
            my -= m_ViewportBounds[0].y;
            // viewport���ڵ����½Ǿ���λ��-���Ͻǵľ���λ��=viewport���ڵĴ�С
            glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
            // ��תy,ʹ�����½ǿ�ʼ����(0,0)
            my = viewportSize.y - my;
            int mouseX = (int)mx;
            int mouseY = (int)my;

            // ���õ�����ʱѡ��ʵ��
            if (mouseX >= 0 && mouseY >= 0 && mouseX < viewportSize.x && mouseY < viewportSize.y)
            {
                // ��ȡ֡����ڶ���������������
                int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
                // TODO����shader��û�����EntityID�������������һ���д���ID��Entity�����޷���ȡ�ᱨ��
                m_HoveredEntity = pixelData == -1 ? Entity() : Entity({ (entt::entity)pixelData, m_ActiveScene.get() });
            
            }


            // ���ǲ�
            OnOverlayRender();

            m_Framebuffer->Unbind();
        }

        Renderer::SetDepthTest(false);
        bool horizontal = true, first_iteration = true;
        uint32_t amount = 10;
        Renderer::GetShaderLibrary()->Get("Blur")->Bind();
        m_BlurFramebuffer[horizontal]->Bind();
        Renderer::Clear();
        m_BlurFramebuffer[!horizontal]->Bind();
        Renderer::Clear();
        for (uint32_t i = 0; i < amount; i++)
        {
            m_BlurFramebuffer[horizontal]->Bind();
            s_BlurUniformBuffer->SetData(&horizontal, sizeof(float));
            uint32_t bloomTextureID = m_Framebuffer->GetColorAttachmentRendererID(2);
            Texture::Bind(first_iteration ? bloomTextureID : m_BlurFramebuffer[!horizontal]->GetColorAttachmentRendererID(0), 0);
            Renderer::DrawIndexed(windowVa, windowVa->GetIndexBuffer()->GetCount());
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        m_BlurFramebuffer[0]->Unbind();
        Renderer::SetDepthTest(true);



        s_ExposureUniformBuffer->SetData(&s_Exposure, sizeof(float));
        s_ExposureUniformBuffer->SetData(&s_Bloom, sizeof(bool), sizeof(float));
        m_Framebuffer->Bind();
        {
            m_HDRShader->Bind();
            uint32_t hdrTextureID = m_Framebuffer->GetColorAttachmentRendererID(0);
            Texture::Bind(hdrTextureID, 0);
            uint32_t blurTextureID = m_BlurFramebuffer[!horizontal]->GetColorAttachmentRendererID(0);
            Texture::Bind(blurTextureID, 1);
            Renderer::SetDepthTest(false);
            Renderer::DrawIndexed(windowVa, windowVa->GetIndexBuffer()->GetCount());
            Renderer::SetDepthTest(true);


            m_Framebuffer->Unbind();

        }







        //=============================================================================================================


        Renderer::Clear();
        // ��FrameBuffer��ͼ��ŵ�window��
        // ��ԭ��ͼ�ߴ�
        Application::Get().GetWindow().ResetViewport();
        // TODO: ֡���廭��ᱻ��������ͼ�ߴ磬�ο�ShaderToy���뽫���汣�������ߴ�
        m_WindowShader->Bind();
        //uint32_t windowTextureID = m_Framebuffer->GetColorAttachmentRendererID(2);
        //uint32_t windowTextureID = m_SpotDepthMapFramebuffer->GetDepthAttachmentRendererID();
        //uint32_t windowTextureID = m_Framebuffer->GetColorAttachmentRendererID(0);
        uint32_t windowTextureID = m_BRDFLUT->GetRendererID();
        Texture::Bind(windowTextureID, 0);
        Renderer::SetDepthTest(false);
        Renderer::DrawIndexed(windowVa, windowVa->GetIndexBuffer()->GetCount());
        Renderer::SetDepthTest(true);


        // Particle Scene
        /*
        Renderer2D::BeginScene(m_CameraEntity.GetComponent<CameraComponent>().Camera, m_CameraEntity.GetComponent<CameraComponent>().Camera.GetProjection());
        {
            //������¿ո�
            if (Input::IsMouseButtonPressed(VOL_MOUSE_BUTTON_LEFT))
            {
                auto [x, y] = Input::GetMousePosition();
                auto width = Application::Get().GetWindow().GetWidth();
                auto height = Application::Get().GetWindow().GetHeight();

                auto bounds = m_CameraController.GetBounds();
                auto pos = m_CameraController.GetCamera().GetPosition();
                x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
                y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
                m_Particle.Position = { x + pos.x, y + pos.y };
                for (int i = 0; i < 5; i++)
                    m_ParticleSystem.Emit(m_Particle);
            }
            m_ParticleSystem.OnUpdate(ts);
            m_ParticleSystem.OnRender();
            Renderer2D::EndScene();
        }
        */
    }

    void ExampleLayer::OnImGuiRender()
    {
        // =============================DockSpace from ImGui::ShowDemoWindow()======================
        static bool p_open = true;

        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        //��ʹ��ImGuiDockNodeFlags_PassthruCentralNodeʱ��DockSpace��������Ⱦ���ǵı���������ͨ�ף��������Ҫ��Begin��������Ⱦ������
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        //��Ҫ��ʾ����ע�⣬��ʹBegin��������false���������۵���������Ҳ������� 
        //������Ϊ�����뱣��DockSpace�������ڻ״̬�����DockSpace�������ڷǻ״̬�� 
        //����ͣ�������еĻ���ڶ���ʧȥ�丸���ڲ�ȡ��ͣ���� 
        //�����޷���������ںͷǻͣ��֮���ͣ����ϵ������ 
        //ͣ���ռ�/���õ��κθ��Ķ��ᵼ�´���������������Զ���ɼ���
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", &p_open, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = minWinSizeX;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                //����ȫ�����������ƶ����������ڵ�ǰ�棬���û�и���ϸ�Ĵ������/z���ƣ�����Ŀǰ�޷�������
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
					OpenProject();

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				ImGui::Separator();

                if (ImGui::MenuItem("Exit")) 
                    Application::Get().Close();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Script"))
            {
                if (ImGui::MenuItem("Reload assembly", "Ctrl+R"))
                    ScriptEngine::ReloadAssembly();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // =====================================================Viewport=====================================================
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");

        // ��ȡViewport�ӿ����Ͻ���viewport�ӿڱ����������ƫ��λ�ã�0, 24) - ������⣬��Ϊ������������ӿڵ����Ͻ�
        auto viewportOffset = ImGui::GetCursorPos();  // Include Tab bar

        // �����Ƿ�ѡ��
        m_ViewportFocused = ImGui::IsWindowFocused();
        // ����Ƿ������ڴ�����
        m_ViewportHovered = ImGui::IsWindowHovered();
        // ֻ��ͬʱѡ�к������Ż�ִ��Event
        Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused);

        // ��ȡ��ͼ���ڳߴ�
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        // ����ͼ��ʾ֡���廭��
        //uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{0, 1}, ImVec2{1, 0});
        
        
        // �����ڴ��ӿ��ϷŹ�����ֵ��On target candidates���Ϸ�Ŀ��
        if (ImGui::BeginDragDropTarget()) 
        {
            // ��Ϊ�������ݿ���Ϊ�գ���Ҫif�жϡ� CONTENT_BROWSER_ITEM���϶�Я��������
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) 
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                OpenScene(path);
            }
            ImGui::EndDragDropTarget();
        }


        // ��ȡvieport�ӿڴ�С - �����������ĸ�
        auto windowSize = ImGui::GetWindowSize();
        // ��ȡ��ǰvieport�ӿڱ��������ϽǾ��뵱ǰ������Ļ���Ͻǣ�0,0����λ��
        ImVec2 minBound = ImGui::GetWindowPos();
        minBound.x += viewportOffset.x;
        minBound.y += viewportOffset.y;

        ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
        // �������ϽǺ����½Ǿ���������Ļ���Ͻǵ�λ��
        m_ViewportBounds[0] = { minBound.x, minBound.y };
        m_ViewportBounds[1] = { maxBound.x, maxBound.y };

        //Gizmos
        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity && m_GizmoType != -1 && m_SceneState == SceneState::Edit)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
            
            //Editor Camera
            const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
            glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

            // Selected Entity transform
            auto& tc = selectedEntity.GetComponent<TransformComponent>();
            glm::mat4 transform = tc.GetTransform();

            // Snapping
            bool snap = Input::IsKeyPressed(Key::LeftControl);
            // Snap to 0.5m for translation/scale
            float snapValue = 0.5f;
            // Snap to 45 defrees for rotation
            if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) 
            {
                snapValue = 45.0f;
            }
            float snapValues[3] = { snapValue, snapValue, snapValue };

            ImGuizmo::Manipulate(
                glm::value_ptr(glm::inverse(cameraView)), 
                glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, 
                ImGuizmo::LOCAL, 
                glm::value_ptr(transform),
                nullptr,
                snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                // Ԥ�����������
                glm::vec3 translation, rotation, scale;
                Math::DecomposeTransform(transform, translation, rotation, scale);

                glm::vec3 deltaRotation = rotation - tc.Rotation;
                tc.Translation = translation;
                tc.Rotation += deltaRotation;
                tc.Scale = scale;
            }
        }

        // Viewport
        ImGui::End();
        ImGui::PopStyleVar();

        m_SceneHierarchyPanel.OnImGuiRender();
        m_ContentBrowserPanel->OnImGuiRender();

        // =====================================================Settings=====================================================
        ImGui::Begin("Stats");

        std::string name = "None";
        if (m_HoveredEntity)
        {
            // BUG!!! �����ڱ����m_HoveredEntityΪ���һ����GetComponent
            //name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
        }
        ImGui::Text("Hovered Entity: %s", name.c_str());

        auto stats = Renderer2D::GetStats();
        ImGui::Text("Renderer2D Stats:");
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);
        ImGui::Text("Vertices: %d", stats.GetTotalVartexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

        for (auto& result : m_ProfileResults)
        {
            char label[50];
            strcpy(label, "%.3fms  ");
            strcat(label, result.Name);
            ImGui::Text(label, result.Time);
        }
        m_ProfileResults.clear();

        // Stats
        ImGui::DragFloat("Exposure", &s_Exposure, 0.01f);
        ImGui::Checkbox("Bloom", &s_Bloom);

        ImGui::DragInt("KernelSize",   &s_KernelSize);
        ImGui::DragFloat("Radius",     &s_Radius,     0.01f);
        ImGui::DragFloat("Bias",       &s_Bias,       0.01f);
        ImGui::DragFloat("Power",      &s_Power,      0.01f);


            
            
            
            


        ImGui::End();

        ImGui::Begin("Settings");
        ImGui::Checkbox("Show physics colliders", &m_ShowPhysicsColliders);
        ImGui::End();

        //ImGui::ShowDemoWindow();

        UI_Toolbar();

        // Dockspace
        ImGui::End();

    }

    void ExampleLayer::UI_Toolbar()
    { 
        // padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        // ��ťͼƬ͸������
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto& colors = ImGui::GetStyle().Colors;
        // ��ť���hover��click�в�ͬЧ��
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        bool toolbarEnabled = (bool)m_ActiveScene;

        ImVec4 tintColor = ImVec4(1, 1, 1, 1);
        if (!toolbarEnabled)
            tintColor.w = 0.5f;

        // ��ť��Ӧ���ڴ�С���Ŵ�ʱӦʹ�����Բ�ֵ��֤����ôģ��
        float size = ImGui::GetWindowHeight() - 4.0f;
        // ��ťӦ�����м�, ���ð�ť��xλ��
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - size);

        bool hasPlayButton     = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play;
        bool hasSimulateButton = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate;
        bool hasPauseButton    = m_SceneState != SceneState::Edit;
        
        // ��ʼ��ͣ��ť
        if(hasPlayButton)
        {
            Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_IconPlay : m_IconStop;
            if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
            {
                if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
                    OnScenePlay();
                else if (m_SceneState == SceneState::Play)
                    OnSceneStop();
            }
        }

        if (hasSimulateButton)
        {
            if(hasPlayButton)
            // ģ�ⰴť
                ImGui::SameLine();
            Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? m_IconSimulate : m_IconStop;

            if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
            {

                if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play)
                    OnSceneSimulate();
                else if (m_SceneState == SceneState::Simulate)
                    OnSceneStop();
            }
        }

        if (hasPauseButton)
        {
            bool isPaused = m_ActiveScene->IsPaused();
            ImGui::SameLine();
            {
                Ref<Texture2D> icon = isPaused ? m_IconPlay : m_IconPause;
                if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
                {
                    m_ActiveScene->SetPaused(!isPaused);
                }
            }

            // Step button
            if (isPaused)
            {
                ImGui::SameLine();
                {
                    Ref<Texture2D> icon = m_IconStep;
                    if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
                    {
                        m_ActiveScene->Step();
                    }
                }
            }

        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);



        ImGui::End();
    }

    void ExampleLayer::OnEvent(Event& event)
    {
        if (m_SceneState == SceneState::Edit)
        {
            m_EditorCamera.OnEvent(event);
        }
        
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(VOL_BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(VOL_BIND_EVENT_FN(ExampleLayer::OnMouseButtonPressed));
        //dispatcher.Dispatch<MouseMovedEvent>(VOL_BIND_EVENT_FN(ExampleLayer::OnMouseMoved));

    }

    bool ExampleLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.IsRepeat()) 
        {
            return false;
        }

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

        switch (e.GetKeyCode()) 
        {
        case Key::N:
            if (control)
                NewScene();
            break;
        case Key::O:
            if (control)
                OpenProject();
            break;
        case Key::S:
            if (control)
                if (shift)
                    SaveSceneAs();
                else
                    SaveScene();
            break;
        case Key::D:
            if (control)
                OnDuplicateEntity();

        //Gizmos
        case Key::Q:
            m_GizmoType = -1;
            break;
        case Key::W:
            m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
            break;
        case Key::E:
            m_GizmoType = ImGuizmo::OPERATION::ROTATE;
            break;
        case Key::R:
            if (control)
            {
                ScriptEngine::ReloadAssembly();
            }
            else
            {
                if (!ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::SCALE;
            }
            break;

            // TODO: ֻ�������viewport��ʵ��ᱻѡ��ɾ����SceneHierarchyPanelѡ��ʵ��delete����ɾ��
        case Key::Delete:
            if (Application::Get().GetImGuiLayer()->GetActiveWidgetID() == 0)
            {
                Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
                if (selectedEntity)
                {
                    m_SceneHierarchyPanel.SetSelectedEntity({});
                    m_ActiveScene->DestroyEntity(selectedEntity);
                }
            }
            break;
        }

        return false;
    }

    bool ExampleLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonLeft)
        {
            // ���������û��Alt
            if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
                m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
        }
        return false;
    }

    /*
    bool ExampleLayer::OnMouseMoved(MouseMovedEvent& e)
    {
        MouseBuffer& mouse = MouseBuffer::instance();

        if (mouse.GetOnActive()) {
            if (mouse.GetFirstMouse())
            {
                mouse.SetLastX(e.GetX());
                mouse.SetLastY(e.GetY());
                mouse.SetFirstMouse(false);
            }

            float xoffset = e.GetX() - mouse.GetLastX();
            float yoffset = mouse.GetLastY() - e.GetY();
            mouse.SetLastX(e.GetX());
            mouse.SetLastY(e.GetY());

            float sensitivity = 0.05;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            mouse.SetYaw(mouse.GetYaw() + xoffset);
            mouse.SetPitch(mouse.GetPitch() + yoffset);
        }
    
    }*/

    void ExampleLayer::OnOverlayRender()
    {
        // ����״̬��
        if (m_SceneState == SceneState::Play)
        {
            Entity camera = m_ActiveScene->GetPrimaryCameraEntity();
            if (!camera)
                return;  // �Ҳ������˳�
            Renderer2D::BeginScene(camera.GetComponent<CameraComponent>().Camera, camera.GetComponent<TransformComponent>().GetTransform());
        }
        else
        {
            Renderer2D::BeginScene(m_EditorCamera, m_EditorCamera.GetViewMatrix());
        }

        if (m_ShowPhysicsColliders)
        {
            // ��Χ��������Ӧ������,��Χ�е�transform����������ƽ�ơ���ת�����š�
            // Box Colliders
            {
                auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
                for (auto entity : view)
                {
                    auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

                    // 0.001f Z��ƫ����
                    glm::vec3 translation = tc.Translation + glm::vec3(bc2d.Offset, 0.001f);
                    // bc2d.Size�����2��������Сһ��
                    glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                        * glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
                        * glm::scale(glm::mat4(1.0f), scale);

                    // ��ɫ�İ�Χ��
                    Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
                }
            }

            // Circle Colliders
            {
                auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                for (auto entity : view)
                {
                    auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

                    glm::vec3 translation = tc.Translation + glm::vec3(cc2d.Offset, 0.001f);
                    // cc2d.Radius�����2��������Сһ��
                    glm::vec3 scale = tc.Scale * glm::vec3(cc2d.Radius * 2.0f);

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                        * glm::scale(glm::mat4(1.0f), scale);

                    Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.01f);
                }
            }
        }

        // Draw selected entity outline 
        if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity())
        {
            const TransformComponent& transform = selectedEntity.GetComponent<TransformComponent>();
            Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); 
        }

        Renderer2D::EndScene();
    }

    void ExampleLayer::NewProject()
    {
        Project::New();
    }

    void ExampleLayer::OpenProject(const std::filesystem::path& path)
    {
        if (Project::Load(path))
        {
            ScriptEngine::Init();
            auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene);
            OpenScene(startScenePath);
            m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
        }
    }

    bool ExampleLayer::OpenProject()
    {
        // ��.hproj�ļ���ȡProject
        std::string filepath = FileDialogs::OpenFile("Volcano Project (*.hproj)\0*.hproj\0");
        if (filepath.empty())
            return false;

        OpenProject(filepath);
        return true;
    }

    void ExampleLayer::SaveProject()
    {
        // Project::SaveActive();
    }

    void ExampleLayer::NewScene()
    {
        m_EditorScene = CreateRef<Scene>();
        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_EditorScenePath = std::filesystem::path();
        m_ActiveScene = m_EditorScene;
    }

    void ExampleLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("Volcano Scene (*.volcano)\0*.volcano\0");
        if (!filepath.empty())
            OpenScene(filepath);
    }
    
    void ExampleLayer::OpenScene(const std::filesystem::path& path)
    {
        if (m_SceneState != SceneState::Edit)
            OnSceneStop();

        if(path.extension().string() != ".volcano")
        {
            VOL_WARN("Could not load {0} - not a scene file", path.filename().string());
            return;
        }

        Ref<Scene> newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(path.string()))
        {
            // �༭��������ȡ�ļ���ȡ����
            m_EditorScene = newScene;
            m_SceneHierarchyPanel.SetContext(m_EditorScene);
            m_EditorScenePath = path;
            m_ActiveScene = m_EditorScene;
        }
    }

    void ExampleLayer::SaveScene()
    {
        if (!m_EditorScenePath.empty())
        {
            SerializeScene(m_ActiveScene, m_EditorScenePath);
            m_EditorScene = m_ActiveScene;
            if (m_SceneState != SceneState::Edit)
                OnSceneStop();
        }
        else
            SaveSceneAs();
    }

    void ExampleLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("Volcano Scene (*.volcano)\0*.volcano\0");
        if (!filepath.empty())
        {
            SerializeScene(m_ActiveScene, filepath);
            m_EditorScenePath = filepath;
            m_EditorScene = m_ActiveScene;
            if (m_SceneState != SceneState::Edit)
                OnSceneStop();
        }
    }

    // ���л�����
    void ExampleLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
    {
        SceneSerializer serializer(scene);
        serializer.Serialize(path.string());
    }

    // ���ų���
    void ExampleLayer::OnScenePlay()
    {
        // ����ʱ����ģ�������ó���
        if (m_SceneState == SceneState::Simulate)
            OnSceneStop();

        // ���ó���״̬������
        m_SceneState = SceneState::Play;

        // �ָ�����Ϊ������Scene
        m_ActiveScene = Scene::Copy(m_EditorScene);

        if (m_SceneHierarchyPanel.GetSelectedEntity())
        {
            UUID selectedEntityID = m_SceneHierarchyPanel.GetSelectedEntity().GetUUID();
            m_SceneHierarchyPanel.SetSelectedEntity(m_ActiveScene->GetEntityByUUID(selectedEntityID));
        }

        m_ActiveScene->SetRunning(true);

        // ���û��������Ч��
        m_ActiveScene->OnRuntimeStart();

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void ExampleLayer::OnSceneSimulate()
    {
        // ����ģ��ʱ���������ó���
        if (m_SceneState == SceneState::Play)
            OnSceneStop();

        // ���ó���״̬��ģ��
        m_SceneState = SceneState::Simulate;

        m_ActiveScene = Scene::Copy(m_EditorScene);

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);

        m_ActiveScene->SetRunning(true);

        m_ActiveScene->OnSimulationStart();

    }

    // ��ͣ����
    void ExampleLayer::OnScenePause()
    {
        VOL_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

        if (m_SceneState == SceneState::Edit)
            return;
        m_ActiveScene->SetPaused(true);
        m_ActiveScene->SetRunning(false);
    }

    // ���ó���
    void ExampleLayer::OnSceneStop()
    {
        VOL_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

        // �رջ��������Ч��
        if (m_SceneState == SceneState::Play)
            m_ActiveScene->OnRuntimeStop();
        else if (m_SceneState == SceneState::Simulate)
            m_ActiveScene->OnSimulationStop();

        // ���ó���״̬���༭
        m_SceneState = SceneState::Edit;

        m_SceneHierarchyPanel.SetContext(m_EditorScene);

        // ������ָ��༭������
        m_ActiveScene = m_EditorScene;

        // editģʽ��running��Ȼfalse����ɾ��
        m_ActiveScene->SetRunning(false);

        MouseBuffer::instance().SetOnActive(true);
    }

    // �༭ģʽ���ã����ѡ��ʵ�壬��ʵ�帴��
    void ExampleLayer::OnDuplicateEntity()
    {
        if (m_SceneState != SceneState::Edit)
            return;

        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity)
        {
            Entity newEntity = m_ActiveScene->DuplicateEntity(selectedEntity);
            m_SceneHierarchyPanel.SetSelectedEntity(newEntity);
        }
    }


}