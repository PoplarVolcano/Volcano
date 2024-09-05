
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
		"%{IncludeDir.assimp}",
		"%{IncludeDir.entt}"
	}

--	postbuildcommands --需要premake在生成项目时执行的命令,把VolcanoNut的asset复制到目标assets
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
		runtime "Debug"
		symbols "On"
		
		links
		{
			"../Volcano/vendor/assimp/lib/Debug/assimp-vc143-mtd.lib"
		}

		postbuildcommands	
		{
			'{COPY} "../Volcano/vendor/assimp/bin/Debug/assimp-vc143-mtd.dll" "%{cfg.targetdir}"'
		}

	filter "configurations:Release"
		defines "VOL_RELEASE"
		runtime "Release"
		symbols "On"
		
		links
		{
			"../Volcano/vendor/assimp/lib/Release/assimp-vc143-mt.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Volcano/vendor/assimp/bin/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"'
		}

	filter "configurations:Dist"
		defines "VOL_DIST"
		runtime "Release"
		symbols "On"
		
		links
		{
			"../Volcano/vendor/assimp/lib/Release/assimp-vc143-mt.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Volcano/vendor/assimp/bin/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"'
		}
