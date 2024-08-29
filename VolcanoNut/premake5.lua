
project "VolcanoNut"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	links 
	{ 
		"Volcano"
	}

	files 
	{ 
		"src/**.h", 
		"src/**.cpp" 
	}
	
	includedirs 
	{
		"%{wks.location}/VolcanoNut/src",
		"%{wks.location}/Volcano/src",
		"%{wks.location}/Volcano/vendor",
		"%{wks.location}/Volcano/vendor/spdlog/include",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.entt}"
	}

--	postbuildcommands --��Ҫpremake��������Ŀʱִ�е�����,��VolcanoNut��asset���Ƶ�Ŀ��assets
--	{
--		'{COPY} "../assets" "%{cfg.targetdir}/assets"'
--	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"VOL_PLATFORM_WINDOWS"
		}
		
	filter "configurations:Debug"
		defines "VOL_DEBUG"
		symbols "On"
		
	filter "configurations:Release"
		defines "VOL_RELEASE"
		symbols "On"
		
	filter "configurations:Dist"
		defines "VOL_DIST"
		symbols "On"
		
