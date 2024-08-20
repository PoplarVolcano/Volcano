include "Dependencies.lua"

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


group "Dependencies"
	include "Volcano/vendor/GLFW"
	include "Volcano/vendor/Glad"
	include "Volcano/vendor/imgui"
	include "Volcano/vendor/yaml-cpp"
group ""

group "Core"
	include "Volcano"
	include "VolcanoNut"
group ""
