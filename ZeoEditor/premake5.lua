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
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.magic_enum}",
		"%{IncludeDir.IconFontCppHeaders}",
		"%{IncludeDir.doctest}"
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
		defines "ZE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "ZE_DIST"
		runtime "Release"
		optimize "on"
