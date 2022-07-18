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

include "ZeoEngine"
include "ZeoEditor"
include "Sandbox"
include "TestBox"
