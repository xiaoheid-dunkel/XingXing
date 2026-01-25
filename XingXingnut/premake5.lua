project "XingXingnut"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/XingXing/vendor/spdlog/include",
        "%{wks.location}/XingXing/src",
        "%{wks.location}/XingXing/vendor",
        "%{IncludeDir.entt}",
        "%{IncludeDir.filewatch}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.ImGuizmo}"
    }

    links
    {
        "XingXing"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "XX_DEBUG"
        runtime "Debug"     -- 👈 恢复为 Debug
        symbols "on"

    filter "configurations:Release"
        defines "XX_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "XX_DIST"
        runtime "Release"
        optimize "on"