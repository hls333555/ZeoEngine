workspace "HBestEngine"
	architecture "x64"
	startproject "Sandbox"

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
IncludeDir["glm"] = "HBestEngine/vendor/glm"
IncludeDir["stb_image"] = "HBestEngine/vendor/stb_image"

-- Include the premake file of GLFW
include "HBestEngine/vendor/GLFW"
-- Include the premake file of Glad
include "HBestEngine/vendor/Glad"
-- Include the premake file of ImGui
include "HBestEngine/vendor/ImGui"

project "HBestEngine"
	location "HBestEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-Intermediate/" .. outputdir .. "/%{prj.name}")

	pchheader "HBEpch.h"
	pchsource "HBestEngine/src/HBEpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.ini"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"HBE_PLATFORM_WINDOWS",
			"HBE_BUILD_DLL",
			-- If this is defined, glfw3.h will not include gl.h which conflicts with glad.h
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "HBE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "HBE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "HBE_DIST"
		runtime "Release"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"HBestEngine/src",
		"HBestEngine/vendor",
		"%{IncludeDir.glm}"
	}

	links
	{
		"HBestEngine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"HBE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "HBE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "HBE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "HBE_DIST"
		runtime "Release"
		optimize "on"
