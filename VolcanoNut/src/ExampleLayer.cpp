#include "ExampleLayer.h"
#include <glm/ext/matrix_transform.hpp>
#include "Volcano/Renderer/Renderer2D.h"

#include <chrono>
#include <imgui/imgui.h>

static const uint32_t s_MapWidth = 24;
static const char* s_MapTiles = 
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWDDDDDDWWWWWWWWWWW"
"WWWWWDDDDDDDDDDWWWWWWWWW"
"WWWWDDDDDDDDDDDDDDWWWWWW"
"WWWDDDDDDDDDDDDDBDDDWWWW"
"WWDDDDDDDDDDDDDDDDDDWWWW"
"WDDDDDWWWDDDDDDDDDDDDWWW"
"WWDDDDWWWDDDDDDDDDDDWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWWDDDDDDDDDDDDDDWWWWWW"
"WWWWWDDDDDDDDDDDDWWWWWWW"
"WWWWWWWDDDDDDDDWWWWWWWWW"
"WWWWWWWWWDDDDWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW";

namespace Volcano{

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
            //销毁对象时，如果计时器尚未停止 ，它会记录结束时间点
            auto endTimepoint = std::chrono::high_resolution_clock::now();
            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
            m_Stopped = true;
            float duration = (end - start) * 0.001f;// 单位ms
            m_Func({ m_Name, duration });
            //std::cout << "Timer:"<< m_Name << "时差：" << duration << "ms" << std::endl;
        }
    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
        bool m_Stopped;
        Fn m_Func;
    };

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); })

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
        m_AlterTexture = Texture2D::Create("assets/textures/莫斯提马.png");
        m_SpriteSheet = Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

        m_TextureStairs = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 6 },  { 128, 128 });
        m_TextureTree = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });

        m_MapWidth = s_MapWidth;
        m_MapHeight = strlen(s_MapTiles) / s_MapWidth;
        s_TextureMap['D'] = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 6, 11}, {128, 128});
        s_TextureMap['W'] = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 11, 11 }, { 128, 128 });

        // Flames
        m_Particle.Position = { 0.0f, 0.0f };
        m_Particle.Velocity = { 0.0f, 0.0f }, m_Particle.VelocityVariation = { 1.0f, 1.0f };
        m_Particle.SizeBegin = 0.2f, m_Particle.SizeEnd = 0.0f, m_Particle.SizeVariation = 0.3f;
        m_Particle.ColorBegin = { 176 / 255.0f, 243 / 255.0f, 253 / 255.0f, 1.0f };
        m_Particle.ColorEnd = { 22 / 255.0f, 58 / 255.0f, 142 / 255.0f , 1.0f };
        m_Particle.LifeTime = 1.0f;

        m_CameraController.SetZoomLevel(5.5f);
    }

    void ExampleLayer::OnDetach()
    {
    }

    void ExampleLayer::OnUpdate(Timestep ts)
    {
        PROFILE_SCOPE("ExampleLayer::OnUpdate");

        Renderer2D::ResetStats();

        {
            PROFILE_SCOPE("m_CameraController::OnUpdate");
            m_CameraController.OnUpdate(ts);
        }

        {
            PROFILE_SCOPE("Renderer Prep");
            Renderer::Clear(0.2f, 0.2f, 0.2f, 1);
        }

        {
            PROFILE_SCOPE("Renderer Draw");

            static float rotation = 0.0f;
            rotation += ts * 20.0f;

            // Background Scene

            Renderer2D::BeginScene(m_CameraController.GetCamera());
            {
                for (float y = -5.0f; y < 5.0f; y += 0.5f)
                {
                    for (float x = -5.0f; x < 5.0f; x += 0.5f)
                    {
                        glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 1.0f };
                        Renderer2D::DrawQuad({ x, y, -0.2f }, { 0.45f, 0.45f }, color);

                    }
                }
                Renderer2D::EndScene();
            }

            // SpriteSheet Scene
            Renderer2D::BeginScene(m_CameraController.GetCamera());
            {
                for (uint32_t y = 0; y < m_MapHeight; y++)
                {
                    for (uint32_t x = 0; x < m_MapWidth; x++)
                    {
                        char tileType = s_MapTiles[x + y * m_MapWidth];
                        Ref<SubTexture2D> texture;
                        if (s_TextureMap.find(tileType) != s_TextureMap.end())
                            texture = s_TextureMap[tileType];
                        else 
                            texture = m_TextureStairs;
                        Renderer2D::DrawQuad({ x - m_MapWidth / 2.0f, y - m_MapHeight / 2.0f, -0.1f }, { 1.0f, 1.0f }, texture);
                    }

                }
                //Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 1.0f, 1.0f }, m_TextureStairs);
                //Renderer2D::DrawQuad({ -1.0f, 0.0f, -0.1f }, { 1.0f, 2.0f }, m_TextureTree);
                Renderer2D::EndScene();
            }

            // Test Scene
            /*
            Renderer2D::BeginScene(m_CameraController.GetCamera());
            {
                glm::vec4  redColor(0.8f, 0.3f, 0.3f, 1.0f);
                glm::vec4  blueColor(0.2f, 0.3f, 0.8f, 1.0f);

                Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.2f }, { 10.0f, 10.0f }, m_Texture);
                Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_AlterTexture);
                Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, redColor);
                Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, blueColor);
                Renderer2D::DrawRotatedQuad({ 1.0f, 0.0f }, { 0.8f,0.8f }, glm::radians(rotation), redColor);
                Renderer2D::DrawRotatedQuad({ -2.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, glm::radians(rotation), m_AlterTexture, 10.0f);
                Renderer2D::EndScene();
            }
            */

            // Particle Scene
            Renderer2D::BeginScene(m_CameraController.GetCamera());
            {
                //如果按下空格
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
        }
    }

    void ExampleLayer::OnImGuiRender()
    {
        ImGui::Begin("Settings");

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
        ImGui::End();
    }

    void ExampleLayer::OnEvent(Event& event)
    {
        m_CameraController.OnEvent(event);
    }
}