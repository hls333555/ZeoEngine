-- ZeoEngine Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["assimp"] = "%{wks.location}/ZeoEngine/vendor/assimp/include"
IncludeDir["Box2D"] = "%{wks.location}/ZeoEngine/vendor/Box2D/include"
IncludeDir["doctest"] = "%{wks.location}/ZeoEngine/vendor/doctest"
IncludeDir["entt"] = "%{wks.location}/ZeoEngine/vendor/entt/include"
IncludeDir["Glad"] = "%{wks.location}/ZeoEngine/vendor/Glad/include"
IncludeDir["GLFW"] = "%{wks.location}/ZeoEngine/vendor/GLFW/include"
IncludeDir["glm"] = "%{wks.location}/ZeoEngine/vendor/glm"
IncludeDir["IconFontCppHeaders"] = "%{wks.location}/ZeoEngine/vendor/IconFontCppHeaders"
IncludeDir["ImGui"] = "%{wks.location}/ZeoEngine/vendor/ImGui"
IncludeDir["ImGuizmo"] = "%{wks.location}/ZeoEngine/vendor/ImGuizmo"
IncludeDir["magic_enum"] = "%{wks.location}/ZeoEngine/vendor/magic_enum/include"
IncludeDir["shaderc"] = "%{wks.location}/ZeoEngine/vendor/shaderc/include"
IncludeDir["spdlog"] = "%{wks.location}/ZeoEngine/vendor/spdlog/include"
IncludeDir["SPIRV_Cross"] = "%{wks.location}/ZeoEngine/vendor/SPIRV-Cross"
IncludeDir["stb_image"] = "%{wks.location}/ZeoEngine/vendor/stb_image"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["yaml_cpp"] = "%{wks.location}/ZeoEngine/vendor/yaml-cpp/include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"