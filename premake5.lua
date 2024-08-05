workspace "Volcano"	--解决方案名称
	architecture "x64"	--编译平台 只编64位
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

--临时变量 定义输出目录 组成输出目录:Debug-windows-x86_64
--详细的所有支持的tokens 可参考 [https://github.com/premake/premake-core/wiki/Tokens]
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

project "Volcano"	--项目名称
	location "Volcano"	--相对路径
	kind "StaticLib"	--表明该项目是lib静态库
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")	--输出目录
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")	--中间临时文件的目录

	-- 预编译头 precompiled header
	pchheader "volpch.h"
	pchsource "Volcano/src/volpch.cpp"

	files	--该项目的文件
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

	includedirs	--附加包含目录
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
	filter "system:windows"	--windows平台的配置
		--cppdialect "C++20"
		-- On:代码生成的运行库选项是MTD,静态链接MSVCRT.lib库;
		-- Off:代码生成的运行库选项是MDD,动态链接MSVCRT.dll库;打包后的exe放到另一台电脑上若无这个dll会报错
		--staticruntime "On"
		systemversion "latest"	-- windowSDK版本

		defines	--预编译宏
		{
			"VOL_PLATFORM_WINDOWS",
			"VOL_BUILD_DLL",
			--"GLFW_INCLUDE_NONE" --让GLFW不包含OpenGL
		}

		--postbuildcommands	-- build后的自定义命令,编译好后移动Volcano.dll文件到Sandbox文件夹下
		--{
		--	("{COPY} %{cfg.buildtarget.relpath} ../bin/" ..outputdir .. "/Sandbox")
		--}

	-- 不同配置下的预定义不同
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