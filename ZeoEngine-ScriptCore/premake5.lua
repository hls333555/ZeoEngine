project "ZeoEngine-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir("%{wks.location}/ZeoEditor/resources/scripts")
	objdir("%{wks.location}/ZeoEditor/resources/scripts/Intermediates")

	pchheader "ZEpch.h"
	pchsource "src/ZEpch.cpp"

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
