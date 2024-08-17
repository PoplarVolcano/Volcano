workspace "Volcano"	--解决方案名称
	architecture "x64"	--编译平台 只编64位
	startproject "VolcanoNut"

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

project "Volcano"	--项目名称
	location "Volcano"	--相对路径
	kind "StaticLib"	--表明该项目是lib静态库
	language "C++"
	cppdialect "C++20"
	-- On:代码生成的运行库选项是MTD,静态链接MSVCRT.lib库;
	-- Off:代码生成的运行库选项是MDD,动态链接MSVCRT.dll库;打包后的exe放到另一台电脑上若无这个dll会报错
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
--		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
--		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"YAML_CPP_STATIC_DEFINE"
	}

	includedirs	--附加包含目录
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

	filter "system:windows"	--windows平台的配置
		systemversion "latest"	-- windowSDK版本

		defines	--预编译宏
		{
			"VOL_PLATFORM_WINDOWS"
		}


	-- 不同配置下的预定义不同
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

	postbuildcommands --需要premake在生成项目时执行的命令,把VolcanoNut的asset复制到目标assets
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

