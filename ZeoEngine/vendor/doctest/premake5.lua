project "doctest"
    kind "StaticLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    files
    {
        "doctest.h",
        "doctest.cpp",
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
		runtime "Debug"
        symbols "on"

	filter "configurations:Release"
		runtime "Release"
        optimize "on"
