include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "XingXing"
    architecture "x86_64"
    startproject "XingXingnut"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    solution_items
    {
        ".editorconfig"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- 恢复标准结构，移除全局的 runtime 强制设置
group "Dependencies"
    include "vendor/premake"
    include "XingXing/vendor/Box2D"
    include "XingXing/vendor/GLFW"
    include "XingXing/vendor/Glad"
    include "XingXing/vendor/msdf-atlas-gen"
    include "XingXing/vendor/imgui"
    include "XingXing/vendor/yaml-cpp"
group ""

group "Core"
    include "XingXing"
    include "XingXing-ScriptCore"
group ""

group "Tools"
    include "XingXingnut"
group ""

group "Misc"
    include "Sandbox"
group ""