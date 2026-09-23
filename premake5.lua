include "Dependencies.lua"

workspace "Volcano" --解决方案
    architecture "x64"  --架构

    startproject "VolcanoEditor" --设置启动项目

    configurations
    {
        "Debug",
        "Release"
    }

-- 输出目录：配置-系统-架构 debug-windows-x64
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" 

group "Dependencies"
    include "volstl"
	include "Volcano/vendor/GLFW"
	include "Volcano/vendor/GLAD"
	include "Volcano/vendor/imgui"
	include "Volcano/vendor/yaml-cpp"
	include "Volcano/vendor/box2d"
group ""

group "Core"
	include "Volcano"
group ""

group "Tools"
	include "VolcanoEditor"
group ""

workspace "VolcanoScript" --解决方案
    architecture "x64"  --架构
    configurations
    {
        "Debug",
        "Release"
    }

include "VolcanoScriptCore"
include "VolcanoScriptApp"
