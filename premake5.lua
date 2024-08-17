workspace "Volcano"	--�����������
	architecture "x64"	--����ƽ̨ ֻ��64λ
	startproject "VolcanoNut"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

--��ʱ���� �������Ŀ¼ ������Ŀ¼:Debug-windows-x86_64
--��ϸ������֧�ֵ�tokens �ɲο� [https://github.com/premake/premake-core/wiki/Tokens]
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Volcano/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Volcano/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Volcano/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Volcano/vendor/glm"
IncludeDir["entt"] = "%{wks.location}/Volcano/vendor/entt/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Volcano/vendor/yaml-cpp/include"
--IncludeDir["ImGuizmo"] = "%{wks.location}/Volcano/vendor/ImGuizmo"

group "Dependencies"
	include "Volcano/vendor/GLFW"
	include "Volcano/vendor/Glad"
	include "Volcano/vendor/imgui"
	include "Volcano/vendor/yaml-cpp"
group ""

project "Volcano"	--��Ŀ����
	location "Volcano"	--���·��
	kind "StaticLib"	--��������Ŀ��lib��̬��
	language "C++"
	cppdialect "C++20"
	-- On:�������ɵ����п�ѡ����MTD,��̬����MSVCRT.lib��;
	-- Off:�������ɵ����п�ѡ����MDD,��̬����MSVCRT.dll��;������exe�ŵ���һ̨�������������dll�ᱨ��
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")	--���Ŀ¼
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")	--�м���ʱ�ļ���Ŀ¼
	
	-- Ԥ����ͷ precompiled header
	pchheader "volpch.h"
	pchsource "Volcano/src/volpch.cpp"

	files	--����Ŀ���ļ�
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
--		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
--		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"YAML_CPP_STATIC_DEFINE"
	}

	includedirs	--���Ӱ���Ŀ¼
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
	--	"%{IncludeDir.ImGuizmo}",
		"%{prj.name}/vendor/assimp/include",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/stb/include"
	}
	
	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib"
	}

--	filter "files:%{prj.name}/vendor/ImGuizmo/**.cpp"
--	flags { "NoPCH" }

	filter "system:windows"	--windowsƽ̨������
		systemversion "latest"	-- windowSDK�汾

		defines	--Ԥ�����
		{
			"VOL_PLATFORM_WINDOWS"
		}


	-- ��ͬ�����µ�Ԥ���岻ͬ
	filter "configurations:Debug"
		defines "VOL_DEBUG"
		symbols "On"
		
	filter "configurations:Release"
		defines "VOL_RELEASE"
		symbols "On"
		
	filter "configurations:Dist"
		defines "VOL_DIST"
		symbols "On"


project "VolcanoNut"
	location "VolcanoNut"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	links 
	{ 
		"Volcano"
	}

	files 
	{ 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.cpp" 
	}
	
	includedirs 
	{
		"%{prj.name}/src",
		"Volcano/vendor",
		"Volcano/src",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
--		"%{IncludeDir.ImGuizmo}",
		"Volcano/vendor/spdlog/include"
	}

	postbuildcommands --��Ҫpremake��������Ŀʱִ�е�����,��VolcanoNut��asset���Ƶ�Ŀ��assets
	{
		'{COPY} "../VolcanoNut/assets" "%{cfg.targetdir}/assets"'
	}
	


	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"VOL_PLATFORM_WINDOWS"
		}
		
	filter "configurations:Debug"
		defines "VOL_DEBUG"
		symbols "On"
		
		links
		{
			"Volcano/vendor/assimp/lib/Debug/assimp-vc143-mtd.lib"
		}

		postbuildcommands	
		{
			'{COPY} "../Volcano/vendor/assimp/bin/Debug/assimp-vc143-mtd.dll" "%{cfg.targetdir}"'
		}
				
	filter "configurations:Release"
		defines "VOL_RELEASE"
		symbols "On"
		
		links
		{
			"Volcano/vendor/assimp/lib/Release/assimp-vc143-mt.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Volcano/vendor/assimp/bin/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"'
		}

	filter "configurations:Dist"
		defines "VOL_DIST"
		symbols "On"
		
		links
		{
			"Volcano/vendor/assimp/lib/Release/assimp-vc143-mt.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Volcano/vendor/assimp/bin/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"'
		}

