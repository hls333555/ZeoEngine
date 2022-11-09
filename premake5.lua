include "Dependencies.lua"

workspace "ZeoEngine"
	architecture "x86_64"
	startproject "ZeoEditor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	defines
	{
		"DOCTEST_CONFIG_DISABLE"
	}

	flags
	{
		"MultiProcessorCompile"
	}

-- Debug-Windows-x64
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include premake files
group "Dependencies"
	include "ZeoEngine/vendor/assimp"
	include "ZeoEngine/vendor/Box2D"
	include "ZeoEngine/vendor/doctest"
	include "ZeoEngine/vendor/GLFW"
	include "ZeoEngine/vendor/Glad"
	include "ZeoEngine/vendor/ImGui"
	include "ZeoEngine/vendor/optick"
	include "ZeoEngine/vendor/yaml-cpp"
group ""

group "Core"
	include "ZeoEngine"
	include "ZeoEngine-ScriptCore"
group ""

group "Tools"
	include "ZeoEditor"
	include "TestBox"
group ""

group "Misc"
	include "Sandbox"
group ""
