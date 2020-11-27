project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-Intermediate/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
	}

	defines
	{
		"ZE_IMVEC_GLMVEC_CONV"
	}

	includedirs
	{
		"%{wks.location}/ZeoEngine/vendor/spdlog/include",
		"%{wks.location}/ZeoEngine/src",
		"%{wks.location}/ZeoEngine/vendor",
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
