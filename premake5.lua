include "Dependencies.lua"

workspace "Volcano"	--�����������
	architecture "x64"	--����ƽ̨ ֻ��64λ
	startproject "VolcanoNut"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

--��ʱ���� �������Ŀ¼ ������Ŀ¼:Debug-windows-x86_64
--��ϸ������֧�ֵ�tokens �ɲο� [https://github.com/premake/premake-core/wiki/Tokens]
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
