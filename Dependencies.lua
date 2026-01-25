-- XingXing Dependencies
-- 已自动适配路径更改和 Vulkan 库修复

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
-- 注意：这里所有的路径前缀都必须是 XingXing，指向你重命名后的文件夹
IncludeDir["stb_image"] = "%{wks.location}/XingXing/vendor/stb_image"
IncludeDir["yaml_cpp"] = "%{wks.location}/XingXing/vendor/yaml-cpp/include"
IncludeDir["Box2D"] = "%{wks.location}/XingXing/vendor/Box2D/include"
IncludeDir["filewatch"] = "%{wks.location}/XingXing/vendor/filewatch"
IncludeDir["GLFW"] = "%{wks.location}/XingXing/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/XingXing/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/XingXing/vendor/ImGui"
IncludeDir["ImGuizmo"] = "%{wks.location}/XingXing/vendor/ImGuizmo"
IncludeDir["glm"] = "%{wks.location}/XingXing/vendor/glm"
IncludeDir["entt"] = "%{wks.location}/XingXing/vendor/entt/include"
IncludeDir["mono"] = "%{wks.location}/XingXing/vendor/mono/include"
IncludeDir["shaderc"] = "%{wks.location}/XingXing/vendor/shaderc/include"
IncludeDir["SPIRV_Cross"] = "%{wks.location}/XingXing/vendor/SPIRV-Cross"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["msdfgen"] = "%{wks.location}/XingXing/vendor/msdf-atlas-gen/msdfgen"
IncludeDir["msdf_atlas_gen"] = "%{wks.location}/XingXing/vendor/msdf-atlas-gen/msdf-atlas-gen"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
-- Mono 库路径也需要指向 XingXing
LibraryDir["mono"] = "%{wks.location}/XingXing/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

-- ✅ Debug 配置：指向标准 Release 库 (解决 LNK1181 找不到 *d.lib 的问题)
Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Tools.lib"

-- Release 配置：保持原样
Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Windows 系统库
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"