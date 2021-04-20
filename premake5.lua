workspace "ZeoEngine"
	architecture "x86_64"
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
IncludeDir["spdlog"] = "%{wks.location}/ZeoEngine/vendor/spdlog/include"
IncludeDir["GLFW"] = "%{wks.location}/ZeoEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/ZeoEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/ZeoEngine/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/ZeoEngine/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/ZeoEngine/vendor/stb_image"
IncludeDir["ImGuizmo"] = "%{wks.location}/ZeoEngine/vendor/ImGuizmo"
IncludeDir["rapidjson"] = "%{wks.location}/ZeoEngine/vendor/rapidjson"
IncludeDir["entt"] = "%{wks.location}/ZeoEngine/vendor/entt/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/ZeoEngine/vendor/yaml-cpp/include"
IncludeDir["magic_enum"] = "%{wks.location}/ZeoEngine/vendor/magic_enum/include"
IncludeDir["IconFontCppHeaders"] = "%{wks.location}/ZeoEngine/vendor/IconFontCppHeaders"
IncludeDir["doctest"] = "%{wks.location}/ZeoEngine/vendor/doctest"

-- Include premake files
group "Dependencies"
	include "ZeoEngine/vendor/GLFW"
	include "ZeoEngine/vendor/Glad"
	include "ZeoEngine/vendor/ImGui"
	include "ZeoEngine/vendor/yaml-cpp"
	include "ZeoEngine/vendor/doctest"
group ""

include "ZeoEngine"
include "Sandbox"
include "ZeoEditor"
include "TestBox"
