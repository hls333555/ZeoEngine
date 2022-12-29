ProjectName = "$PROJECT_NAME$"

workspace "%{ProjectName}"
	architecture "x86_64"
	startproject "%{ProjectName}"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

-- Debug-Windows-x64
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "%{ProjectName}"
	location "Assets/Scripts"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetname "%{ProjectName}"
	targetdir("%{prj.location}/Binaries")
	objdir("%{prj.location}/Intermediates")

	files
	{
		"%{prj.location}/Source/**.cs"
	}

	links
	{
		"ZeoEngine-ScriptCore"
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

group "ZeoEngine"
	include "$ENGINE_ROOT$/ZeoEngine-ScriptCore"
group ""
