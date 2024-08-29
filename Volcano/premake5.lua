
project "Volcano"	--项目名称
	kind "StaticLib"	--表明该项目是lib静态库
	language "C++"
	cppdialect "C++20"
	-- On:代码生成的运行库选项是MTD,静态链接MSVCRT.lib库;
	-- Off:代码生成的运行库选项是MDD,动态链接MSVCRT.dll库;打包后的exe放到另一台电脑上若无这个dll会报错
	staticruntime "off"
	-- 多字节字符集
	characterset "MBCS"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")	--输出目录
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")	--中间临时文件的目录
	
	-- 预编译头 precompiled header
	pchheader "volpch.h"
	pchsource "src/volpch.cpp"

	files	--该项目的文件
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"YAML_CPP_STATIC_DEFINE"
	}

	includedirs	--附加包含目录
	{
		"src",
		"vendor",
		"vendor/assimp/include",
		"vendor/spdlog/include",
		"vendor/stb/include",
		"%{IncludeDir.box2d}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.VulkanSDK}"
	}
	
	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"box2d",
		"opengl32.lib",
		"%{Library.mono}"
	}
	
	-- 创建预编译头文件
    filter "files:src/volpch.cpp"
      buildoptions { "/Yc\"volpch.h\"" }

	filter "files:src/Volcano/ImGui/ImGuizmo.cpp"
	flags { "NoPCH" }

	filter "system:windows"	--windows平台的配置
		systemversion "latest"	-- windowSDK版本

		defines	--预编译宏
		{
			"VOL_PLATFORM_WINDOWS"
		}

		links
		{
			"%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}",
		}

	-- 不同配置下的预定义不同
	filter "configurations:Debug"
		defines "VOL_DEBUG"
		runtime "Debug"
		symbols "On"
		
		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}


	filter "configurations:Release"
		defines "VOL_RELEASE"
		runtime "Release"
		symbols "On"
		
		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}
		

	filter "configurations:Dist"
		defines "VOL_DIST"
		runtime "Release"
		symbols "On"
		
		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}
		