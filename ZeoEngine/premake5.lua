project "ZeoEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-Intermediate/" .. outputdir .. "/%{prj.name}")

	pchheader "ZEpch.h"
	pchsource "src/ZEpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.ini",
		"vendor/IconFontCppHeaders/**.h",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE", -- If this is defined, glfw3.h will not include gl.h which conflicts with glad.h
		'IMGUI_USER_CONFIG="Engine/ImGui/MyImGuiConfig.h"'
	}

	includedirs
	{
		"src",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.debug_draw}",
		"%{IncludeDir.doctest}",
		"%{IncludeDir.dylib}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.IconFontCppHeaders}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.magic_enum}",
		"%{IncludeDir.renderdoc}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.yaml_cpp}",
	}

	links 
	{
		"opengl32.lib",
		"assimp",
		"Box2D",
		"doctest",
		"Glad",
		"GLFW",
		"ImGui",
		"yaml-cpp",
	}

	filter "files:vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "ZE_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

	filter "configurations:Release"
		defines "ZE_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	filter "configurations:Dist"
		defines "ZE_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}
