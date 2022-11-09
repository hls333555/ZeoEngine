project "ZeoEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-Intermediate/" .. outputdir .. "/%{prj.name}")

	defines
    {
        'IMGUI_USER_CONFIG="Engine/ImGui/MyImGuiConfig.h"'
    }

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"src",
		"%{wks.location}/ZeoEngine/src",
		"%{IncludeDir.debug_draw}",
		"%{IncludeDir.doctest}",
		"%{IncludeDir.dylib}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.IconFontCppHeaders}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.magic_enum}",
		"%{IncludeDir.optick}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.renderdoc}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.yaml_cpp}",
	}

	links
	{
		"ZeoEngine"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "ZE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

		defines
		{
			"ZE_RELEASE",
			"NDEBUG" -- PhysX Requires This
		}

	filter "configurations:Dist"
		defines "ZE_DIST"
		runtime "Release"
		optimize "on"
