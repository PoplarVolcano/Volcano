
project "volstl"
    kind "SharedLib"
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
        "src"
    }
    
    filter "system:windows"    --仅对 Windows 系统生效后续的配置
        systemversion "latest"
        buildoptions "/utf-8"

        defines
        {
            "VOL_PLATFORM_WINDOWS",
            "VOLSTL_BUILD_DLL"
        }
        
        local wksLocation      = ".."
        local volcanoEditorDir = path.join(wksLocation, "bin", outputdir, "VolcanoEditor")
        local volstlDir       = path.join(wksLocation, "bin", outputdir, "volstl")
        postbuildcommands --将volstl/bin/outputdir/volstl中的.dll文件复制到VolcanoEditor/bin/outputdir/VolcanoEditor中
        {
            'if not exist "' .. volcanoEditorDir .. '" mkdir "' .. volcanoEditorDir .. '"',
            '{COPYFILE} "' .. volstlDir .. '\\*.dll" "' .. volcanoEditorDir .. '"',
        }

    filter "configurations:Debug"
        runtime "Debug"
        defines "VOL_DEBUG"
        symbols "On" --是否生成调试信息（符号文件）
    
    filter "configurations:Release"
        runtime "Release"
        defines "VOL_RELEASE"
        optimize "On" --设置编译器代码优化级别,On：基础优化