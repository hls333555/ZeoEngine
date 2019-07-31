workspace "HBestEngine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

-- Debug-Windows-x64
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "HBestEngine"
	location "HBestEngine"
	kind "SharedLib"
	language "C++"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-Intermediate/" .. outputdir .. "/%{prj.name}")

	pchheader "HBEpch.h"
	pchsource "HBestEngine/src/HBEpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"HBE_PLATFORM_WINDOWS",
			"HBE_BUILD_DLL"
		}

	filter "configurations:Debug"
		defines "HBE_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "HBE_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "HBE_DIST"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-Intermediate/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"HBestEngine/vendor/spdlog/include",
		"HBestEngine/src"
	}

	links
	{
		"HBestEngine"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"HBE_PLATFORM_WINDOWS"
		}

		postbuildcommands
		{
			("{COPY} ../bin/"  .. outputdir .. "/HBestEngine/HBestEngine.dll ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "HBE_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "HBE_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "HBE_DIST"
		optimize "On"
