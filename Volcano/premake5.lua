project "Volcano"
    kind "SharedLib"
    language "C++"
	cppdialect "C++20"
	-- 多字节字符集
	characterset "MBCS"

    --kind "ConsoleApp"       -- 控制台应用程序（.exe）
    --kind "WindowedApp"      -- 窗口GUI程序（.exe，无黑框）
    --kind "StaticLib"        -- 静态库（.lib / .a）
    --kind "SharedLib"        -- 动态库（.dll / .so / .dylib）

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}") --输出目录
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}") --中间目录

    files
    {
        "src/**.h",
        "src/**.inl",
        "src/**.cpp",
        "vendor/stb_image/**.h",
        "vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/ImGuizmo/src/ImGuizmo.h",
		"vendor/ImGuizmo/src/ImGuizmo.cpp"
    }

    defines
    {
		    "YAML_CPP_STATIC_DEFINE",
            "VOL_BUILD_DLL"
    }

    includedirs  --附加包含目录
    {
        "src",
        "vendor",
        "vendor/spdlog/include",
        "vendor/glm",
        "%{wks.location}/volstl/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.box2d}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.VulkanSDK}"
    }

    links
    {
        "GLFW",
        "GLAD",
		"ImGui",
		"yaml-cpp",
		"box2d",
		"%{Library.mono}"
    }

    disablewarnings { "4251", "4267" }

    pchheader "volpch.h"           -- 预编译头文件（.h）
    pchsource "src/volpch.cpp"     -- 预编译头实现文件（.cpp）
    
	filter "files:vendor/ImGuizmo/src/**.cpp"
	flags { "NoPCH" }

    filter "system:windows"    --仅对 Windows 系统生效后续的配置
        systemversion "latest" --windows系统版本
        buildoptions "/utf-8"  --现代库（fmt、range-v3、C++20 等）要求源码 UTF-8
        
        defines --预处理器定义
        {
		    "_CRT_SECURE_NO_WARNINGS",--屏蔽 Visual Studio 编译器（MSVC）关于“不安全”标准 C 库函数的警告（C4996）,
            "VOL_PLATFORM_WINDOWS"
        }
        
		links
		{
			"%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}",
		}

        --将Volcano/bin/outputdir/Volcano中的.dll文件复制到VolcanoEditor/bin/outputdir/VolcanoEditor中
        -- 构建顺序：Volcano 构建 → Volcano的 postbuild
        local wksLocation      = ".."
        local volcanoEditorDir = path.join(wksLocation, "bin", outputdir, "VolcanoEditor")
        local volcanoDir       = path.join(wksLocation, "bin", outputdir, "Volcano")
        local monoLib0         = path.join(wksLocation, "bin", outputdir, "Volcano", "vendor")
        local monoLib1         = path.join(wksLocation, "bin", outputdir, "Volcano", "vendor", "mono")
        local monoLib2         = path.join(wksLocation, "bin", outputdir, "Volcano", "vendor", "mono", "lib")
        local monoLibRoot      = path.join(wksLocation, "bin", outputdir, "lib")
        local monoLibInVolcano = path.join(wksLocation, "Volcano", "vendor", "mono", "lib")
        postbuildcommands
        {
            'if not exist "' .. volcanoEditorDir .. '" mkdir "' .. volcanoEditorDir .. '"',
            '{COPYFILE} "' .. volcanoDir .. '\\*.dll" "' .. volcanoEditorDir .. '"',

            'if not exist "' .. monoLib0 .. '" mkdir "' .. monoLib0 .. '"',
            'if not exist "' .. monoLib1 .. '" mkdir "' .. monoLib1 .. '"',
            'if not exist "' .. monoLib2 .. '" mkdir "' .. monoLib2 .. '"',
            '{COPYDIR} "' .. monoLibInVolcano .. '" "' .. monoLib2 .. '"',

            'if not exist "' .. monoLibRoot .. '" mkdir "' .. monoLibRoot .. '"',
            '{COPYDIR} "' .. monoLibInVolcano .. '" "' .. monoLibRoot .. '"'
        }

    filter "configurations:Debug"
        runtime "Debug"
        defines "VOL_DEBUG"
        symbols "On" --是否生成调试信息（符号文件）
    
		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

    filter "configurations:Release"
        runtime "Release"
        defines "VOL_RELEASE"
        optimize "On" --设置编译器代码优化级别,On：基础优化
        
		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}