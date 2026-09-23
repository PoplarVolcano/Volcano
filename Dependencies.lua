

VULKAN_SDK = "%{wks.location}/Volcano/vendor/VulkanSDK";--os.getenv("VULKAN_SDK")

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"]      = "%{wks.location}/Volcano/vendor/GLFW/include"
IncludeDir["GLAD"]      = "%{wks.location}/Volcano/vendor/Glad/include"
IncludeDir["ImGui"]     = "%{wks.location}/Volcano/vendor/imgui"
IncludeDir["ImGuizmo"]  = "%{wks.location}/Volcano/vendor/ImGuizmo/src"
IncludeDir["glm"]       = "%{wks.location}/Volcano/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/Volcano/vendor/stb_image"
IncludeDir["entt"]      = "%{wks.location}/Volcano/vendor/entt/include"
IncludeDir["yaml_cpp"]  = "%{wks.location}/Volcano/vendor/yaml-cpp/include"
IncludeDir["box2d"]     = "%{wks.location}/Volcano/vendor/box2d/include"
IncludeDir["mono"]      = "%{wks.location}/Volcano/vendor/mono/include"
IncludeDir["filewatch"] = "%{wks.location}/Volcano/vendor/filewatch"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/include"

LibraryDir = {}
LibraryDir["mono"] = "%{wks.location}/Volcano/vendor/mono/lib/%{cfg.buildcfg}"
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/lib"

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