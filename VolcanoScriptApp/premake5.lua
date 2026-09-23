project "VolcanoScriptApp"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.8"

	targetdir ("../VolcanoEditor/Resources/Scripts")
	objdir ("../VolcanoEditor/Resources/Scripts/Intermediates")

	files 
	{
		"Source/**.cs",
		"Properties/**.cs"
	}

	links { "VolcanoScriptCore" } 

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"