workspace "Volcano"	--�����������
	architecture "x64"	--����ƽ̨ ֻ��64λ
	startproject "Sandbox"

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
IncludeDir["stb_image"] = "Volcano/vendor/stb_image"

include "Volcano/vendor/GLFW"
include "Volcano/vendor/Glad"
include "Volcano/vendor/imgui"

project "Volcano"	--��Ŀ����
	location "Volcano"	--���·��
	kind "StaticLib"	--��������Ŀ��lib��̬��
	language "C++"
	cppdialect "C++20"
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
		"%{prj.name}/vendor/glm/glm/**.cppm"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs	--���Ӱ���Ŀ¼
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}
	filter "system:windows"	--windowsƽ̨������
		--cppdialect "C++20"
		-- On:�������ɵ����п�ѡ����MTD,��̬����MSVCRT.lib��;
		-- Off:�������ɵ����п�ѡ����MDD,��̬����MSVCRT.dll��;������exe�ŵ���һ̨�������������dll�ᱨ��
		--staticruntime "On"
		systemversion "latest"	-- windowSDK�汾

		defines	--Ԥ�����
		{
			"VOL_PLATFORM_WINDOWS",
			"VOL_BUILD_DLL",
			--"GLFW_INCLUDE_NONE" --��GLFW������OpenGL
		}

		--postbuildcommands	-- build����Զ�������,����ú��ƶ�Volcano.dll�ļ���Sandbox�ļ�����
		--{
		--	("{COPY} %{cfg.buildtarget.relpath} ../bin/" ..outputdir .. "/Sandbox")
		--}

	-- ��ͬ�����µ�Ԥ���岻ͬ
	filter "configurations:Debug"
		defines "VOL_DEBUG"
		buildoptions "/MDd"
		runtime "Debug"
		symbols "On"
		
	filter "configurations:Release"
		defines "VOL_RELEASE"
		buildoptions "/MD"
		runtime "Release"
		symbols "On"
		
	filter "configurations:Dist"
		defines "VOL_DIST"
		buildoptions "/MD"
		runtime "Release"
		symbols "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "On"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Volcano/vendor/spdlog/include;",
		"Volcano/src",
		"%{IncludeDir.glm}",
		"Volcano/vendor"
	}

	links
	{
		"Volcano"
	}

	filter "system:windows"
		--cppdialect "C++20"
		--staticruntime "On"
		systemversion "latest"
		
		defines
		{
			"VOL_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "VOL_DEBUG"
		buildoptions "/MDd"
		symbols "On"
		
	filter "configurations:Release"
		defines "VOL_RELEASE"
		buildoptions "/MD"
		symbols "On"
		
	filter "configurations:Dist"
		defines "VOL_DIST"
		buildoptions "/MD"
		symbols "On"