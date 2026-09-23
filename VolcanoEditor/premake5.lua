
project "VolcanoEditor"
    kind "ConsoleApp" --控制台应用程序（.exe）
    language "C++"
	cppdialect "C++20"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}") --输出目录
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}") --中间目录

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs  --附加包含目录
    {
        "src",
        "%{wks.location}/Volcano/src",
        "%{wks.location}/Volcano/vendor",
        "%{wks.location}/Volcano/vendor/spdlog/include",
        "%{wks.location}/volstl/src",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
    }
    
    
    links
    {
        "Volcano",
        "volstl"
    }

    dependson
    {
        "Volcano",
        "volstl"
    }

    disablewarnings { "4251", "4267" }

    filter "system:windows" --仅对 Windows 系统生效后续的配置
        systemversion "latest"
        buildoptions "/utf-8"

        defines
        {
            "VOL_PLATFORM_WINDOWS"
        }

        local wksLocation      = ".."
        local VolcanoEditor = path.join(wksLocation, "VolcanoEditor")
        local VolcanoEditorDir = path.join(wksLocation, "bin", outputdir, "VolcanoEditor")
        local ResourcesDir = path.join(wksLocation, "bin", outputdir, "VolcanoEditor", "Resources")
        local ResourcesInVolcanoEditor = path.join(wksLocation, "VolcanoEditor", "Resources")
        postbuildcommands --需要premake在生成项目时执行的命令,把VolcanoNut的Resources文件夹复制到目标Resources
        { 
            '{COPYFILE} "' .. VolcanoEditor .. '\\imgui.ini" "' .. VolcanoEditorDir .. '"',

            'if not exist "' .. ResourcesDir .. '" mkdir "' .. ResourcesDir .. '"',
            '{COPYDIR} "' .. ResourcesInVolcanoEditor .. '" "' .. ResourcesDir .. '"'
        }
	
    filter "configurations:Debug"
        runtime "Debug"
        defines "VOL_DEBUG"
        symbols "On" --是否生成调试信息（符号文件）
    
    filter "configurations:Release"
        runtime "Release"
        defines "VOL_RELEASE"
        optimize "On" --设置编译器代码优化级别,On：基础优化

