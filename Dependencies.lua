
VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["box2d"] = "%{wks.location}/Volcano/vendor/box2d/include"
IncludeDir["GLFW"] = "%{wks.location}/Volcano/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Volcano/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Volcano/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Volcano/vendor/glm"
IncludeDir["entt"] = "%{wks.location}/Volcano/vendor/entt/include"
IncludeDir["mono"] = "%{wks.location}/Volcano/vendor/mono/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Volcano/vendor/yaml-cpp/include"
IncludeDir["shaderc"] = "%{wks.location}/Volcano/vendor/shaderc/include"
IncludeDir["SPIRV_Cross"] = "%{wks.location}/Volcano/vendor/SPIRV-Cross"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"


LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["mono"] = "%{wks.location}/Volcano/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"