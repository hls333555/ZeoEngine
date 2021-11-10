import os
import subprocess
import ssl
import CheckPython

# Make sure everything we need is installed
CheckPython.ValidatePackages()

import Vulkan

# Change from Scripts directory to root
os.chdir('../')

# Prevent [SSL: CERTIFICATE_VERIFY_FAILED] issue
ssl._create_default_https_context = ssl._create_unverified_context

if (not Vulkan.CheckVulkanSDK()):
    print("Vulkan SDK not installed.")

if (not Vulkan.CheckVulkanSDKDebugLibs()):
    print("Vulkan SDK debug libs not found.")

print("Updating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

print("Running premake...")
subprocess.call(["vendor/premake/premake5.exe", "vs2022"])
