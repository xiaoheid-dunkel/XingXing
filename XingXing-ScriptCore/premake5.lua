project "XingXing-ScriptCore"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"

    -- 注意：这里的路径必须指向你重命名后的编辑器资源目录
    targetdir ("../XingXingEditor/Resources/Scripts")
    objdir ("../XingXingEditor/Resources/Scripts/Intermediates")

    files 
    {
        "Source/**.cs",
        "Properties/**.cs"
    }
    
    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"

    filter "configurations:Release"
        optimize "On"
        symbols "Default"

    filter "configurations:Dist"
        optimize "Full"
        symbols "Off"