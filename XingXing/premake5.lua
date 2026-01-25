project "XingXing"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    -- 这里的 prj.name 会自动识别为 "XingXing"
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    -- 如果你打算把 hzpch.h 也改名，记得修改这里。目前保留原名以确保能直接编译。
    pchheader "hzpch.h"
    pchsource "src/hzpch.cpp"

    files
    {
        "src/**.h",
        "src/**.cpp",
        "vendor/stb_image/**.h",
        "vendor/stb_image/**.cpp",
        "vendor/glm/glm/**.hpp",
        "vendor/glm/glm/**.inl",

        "vendor/ImGuizmo/ImGuizmo.h",
        "vendor/ImGuizmo/ImGuizmo.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE"
    }

    includedirs
    {
        "src",
        "vendor/spdlog/include",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.filewatch}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.msdfgen}",
        "%{IncludeDir.msdf_atlas_gen}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.mono}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.VulkanSDK}"
    }

    links
    {
        "Box2D",
        "GLFW",
        "Glad",
        "ImGui",
        "msdf-atlas-gen",
        "yaml-cpp",
        "opengl32.lib",

        "%{Library.mono}",
    }

    filter "files:vendor/ImGuizmo/**.cpp"
        flags { "NoPCH" }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            -- 之后可以将 HZ_PLATFORM_WINDOWS 改为 XX_PLATFORM_WINDOWS
            "HZ_PLATFORM_WINDOWS" 
        }

        links
        {
            "%{Library.WinSock}",
            "%{Library.WinMM}",
            "%{Library.WinVersion}",
            "%{Library.BCrypt}",
        }

    filter "configurations:Debug"
        defines "XX_DEBUG" -- 将 HZ 改为 XX
        runtime "Debug"
        symbols "on"

        links
        {
            "%{Library.ShaderC_Debug}",
            "%{Library.SPIRV_Cross_Debug}",
            "%{Library.SPIRV_Cross_GLSL_Debug}"
        }

    filter "configurations:Release"
        defines "XX_RELEASE"
        runtime "Release"
        optimize "on"

        links
        {
            "%{Library.ShaderC_Release}",
            "%{Library.SPIRV_Cross_Release}",
            "%{Library.SPIRV_Cross_GLSL_Release}"
        }

    filter "configurations:Dist"
        defines "XX_DIST"
        runtime "Release"
        optimize "on"

        links
        {
            "%{Library.ShaderC_Release}",
            "%{Library.SPIRV_Cross_Release}",
            "%{Library.SPIRV_Cross_GLSL_Release}"
        }