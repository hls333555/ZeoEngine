import os
import subprocess
import platform
import ssl

# Prevent [SSL: CERTIFICATE_VERIFY_FAILED] issue
#ssl._create_default_https_context = ssl._create_unverified_context

from SetupPython import PythonConfiguration as PythonRequirements

# Make sure everything we need for the setup is installed
PythonRequirements.Validate()

from SetupVulkan import VulkanConfiguration as VulkanRequirements
# Change from devtools/scripts directory to root
os.chdir('./../')

VulkanRequirements.Validate()

print("\nUpdating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

if platform.system() == "Windows":
    print("\nRunning premake...")
    subprocess.call([os.path.abspath("./scripts/Win-GenProjects.bat"), "nopause"])
