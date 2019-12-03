workspace "ZeoEngine"
	architecture "x64"
	startproject "Sandbox"

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

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "ZeoEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "ZeoEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "ZeoEngine/vendor/imgui"
IncludeDir["glm"] = "ZeoEngine/vendor/glm"
IncludeDir["stb_image"] = "ZeoEngine/vendor/stb_image"

-- Include the premake file of GLFW
include "ZeoEngine/vendor/GLFW"
-- Include the premake file of Glad
include "ZeoEngine/vendor/Glad"
-- Include the premake file of ImGui
include "ZeoEngine/vendor/ImGui"

project "ZeoEngine"
	location "ZeoEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-Intermediate/" .. outputdir .. "/%{prj.name}")

	pchheader "ZEpch.h"
	pchsource "ZeoEngine/src/ZEpch.cpp"

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
			"ZE_PLATFORM_WINDOWS",
			"ZE_BUILD_DLL",
			-- If this is defined, glfw3.h will not include gl.h which conflicts with glad.h
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "ZE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ZE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "ZE_DIST"
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
		"ZeoEngine/vendor/spdlog/include",
		"ZeoEngine/src",
		"ZeoEngine/vendor",
		"%{IncludeDir.glm}"
	}

	links
	{
		"ZeoEngine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"ZE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "ZE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ZE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "ZE_DIST"
		runtime "Release"
		optimize "on"
