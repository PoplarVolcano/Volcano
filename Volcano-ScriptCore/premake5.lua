project "Volcano-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.8.0"

	targetdir ("../VolcanoNut/Resources/Scripts")
	objdir ("../VolcanoNut/Resources/Scripts/Intermediates")

	files 
	{
		"Source/**.cs",
		"Properties/**.cs"
	}

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"