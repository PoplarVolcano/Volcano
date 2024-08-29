
project "Volcano"	--��Ŀ����
	kind "StaticLib"	--��������Ŀ��lib��̬��
	language "C++"
	cppdialect "C++20"
	-- On:�������ɵ����п�ѡ����MTD,��̬����MSVCRT.lib��;
	-- Off:�������ɵ����п�ѡ����MDD,��̬����MSVCRT.dll��;������exe�ŵ���һ̨�������������dll�ᱨ��
	staticruntime "off"
	-- ���ֽ��ַ���
	characterset "MBCS"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")	--���Ŀ¼
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")	--�м���ʱ�ļ���Ŀ¼
	
	-- Ԥ����ͷ precompiled header
	pchheader "volpch.h"
	pchsource "src/volpch.cpp"

	files	--����Ŀ���ļ�
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

	includedirs	--���Ӱ���Ŀ¼
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
	
	-- ����Ԥ����ͷ�ļ�
    filter "files:src/volpch.cpp"
      buildoptions { "/Yc\"volpch.h\"" }

	filter "files:src/Volcano/ImGui/ImGuizmo.cpp"
	flags { "NoPCH" }

	filter "system:windows"	--windowsƽ̨������
		systemversion "latest"	-- windowSDK�汾

		defines	--Ԥ�����
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

	-- ��ͬ�����µ�Ԥ���岻ͬ
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
		