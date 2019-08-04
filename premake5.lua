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

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "HBestEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "HBestEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "HBestEngine/vendor/imgui"

-- Include the premake file of GLFW
include "HBestEngine/vendor/GLFW"
-- Include the premake file of Glad
include "HBestEngine/vendor/Glad"
-- Include the premake file of ImGui
include "HBestEngine/vendor/ImGui"

startproject "Sandbox"

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
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"HBE_PLATFORM_WINDOWS",
			"HBE_BUILD_DLL",
			-- If this is defined, glfw3.h will not include gl.h which conflicts with glad.h
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} ../bin/"  .. outputdir .. "/HBestEngine/HBestEngine.dll ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "HBE_DEBUG"
		-- Multi-threaded Debug Dll
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines "HBE_RELEASE"
		-- Multi-threaded Dll
		buildoptions "/MD"
		optimize "On"

	filter "configurations:Dist"
		defines "HBE_DIST"
		-- Multi-threaded Dll
		buildoptions "/MD"
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

	filter "configurations:Debug"
		defines "HBE_DEBUG"
		-- Multi-threaded Debug Dll
		buildoptions "/MDd"
		symbols "On"

	filter "configurations:Release"
		defines "HBE_RELEASE"
		-- Multi-threaded Dll
		buildoptions "/MD"
		optimize "On"

	filter "configurations:Dist"
		defines "HBE_DIST"
		-- Multi-threaded Dll
		buildoptions "/MD"
		optimize "On"
