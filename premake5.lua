workspace "ZeoEngine"
	architecture "x64"
	startproject "ZeoEditor"

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
IncludeDir["rttr"] = "ZeoEngine/vendor/rttr/src/"
IncludeDir["ImGuizmo"] = "ZeoEngine/vendor/ImGuizmo"
IncludeDir["rapidjson"] = "ZeoEngine/vendor/rapidjson"
IncludeDir["entt"] = "ZeoEngine/vendor/entt/include"
IncludeDir["yaml_cpp"] = "ZeoEngine/vendor/yaml-cpp/include"

-- Include premake files
group "Dependencies"
	include "ZeoEngine/vendor/GLFW"
	include "ZeoEngine/vendor/Glad"
	include "ZeoEngine/vendor/ImGui"
	include "ZeoEngine/vendor/rttr"
	include "ZeoEngine/vendor/yaml-cpp"
group ""

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
		"%{prj.name}/vendor/glm/glm/**.ini",
		"%{prj.name}/vendor/ImGuizmo/**.h",
		"%{prj.name}/vendor/ImGuizmo/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"ZE_IMVEC_GLMVEC_CONV"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.rttr}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.rapidjson}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib",
		"rttr",
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

project "ZeoEditor"
	location "ZeoEditor"
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
		"Sandbox/src/Game/**.h",
		"Sandbox/src/Game/**.cpp",
	}

	defines
	{
		"ZE_IMVEC_GLMVEC_CONV"
	}

	includedirs
	{
		"%{prj.name}/src",
		"ZeoEngine/vendor/spdlog/include",
		"ZeoEngine/src",
		"ZeoEngine/vendor",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.rttr}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.rapidjson}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}"
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

	defines
	{
		"ZE_IMVEC_GLMVEC_CONV"
	}

	includedirs
	{
		"ZeoEngine/vendor/spdlog/include",
		"ZeoEngine/src",
		"ZeoEngine/vendor",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.rttr}",
		"%{IncludeDir.entt}"
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
