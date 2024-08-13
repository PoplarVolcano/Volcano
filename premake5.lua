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
IncludeDir["GLFW"] = "Volcano/vendor/GLFW/include"
IncludeDir["Glad"] = "Volcano/vendor/Glad/include"
IncludeDir["ImGui"] = "Volcano/vendor/imgui"
IncludeDir["glm"] = "Volcano/vendor/glm"

include "Volcano/vendor/GLFW"
include "Volcano/vendor/Glad"
include "Volcano/vendor/imgui"

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
		"%{prj.name}/src/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs	--���Ӱ���Ŀ¼
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{prj.name}/vendor/assimp/include",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/stb/include"
	}
	
	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

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
		"Volcano/vendor/spdlog/include;"
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

