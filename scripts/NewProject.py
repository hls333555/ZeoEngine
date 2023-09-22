import os
import shutil
import subprocess

class NewProject:
    @classmethod
    def Create(cls):
        engineRoot = os.environ.get("ZEOENGINE_DIR")
        if (engineRoot is None):
            print("ZeoEngine is not properly installed! Try rerun the Setup.bat")
            return

        projectName = str(input("Please enter the project name:"))
        projectDirectory = str(input("Please enter the project directory:"))
        projectRoot = projectDirectory + "\\" + projectName

        print("Creating project: " + projectName)
        # Create project root directory
        if (not os.path.exists(projectRoot)):
            os.makedirs(projectRoot)
        
        # Copy project files
        shutil.copytree(engineRoot + "/ZeoEditor/NewProjectTemplate", projectRoot, dirs_exist_ok=True)
        cls.ReplaceProjectName(projectRoot + "/premake5.lua", projectName)
        cls.ReplaceEngineRootDirectory(projectRoot + "/premake5.lua", engineRoot)
        projectFilePath = ("{}/{}.zproject").format(projectRoot, projectName)
        os.rename(projectRoot + "/NewProject.zproject", projectFilePath)
        cls.ReplaceProjectName(projectFilePath, projectName)
        os.chdir(projectRoot)
        # Create assets folder
        os.makedirs("Assets", exist_ok=True)

        # Generating scripts solution file
        subprocess.call([os.path.abspath(projectRoot + "/Win-GenScriptProjects.bat")])

    @classmethod
    def ReplaceProjectName(cls, path, projectName):
        projectNameToken = "$PROJECT_NAME$"
        with open(path, 'r',encoding='UTF-8') as file:
            data = file.read()
            data = data.replace(projectNameToken, projectName)
        with open(path, 'w',encoding='UTF-8') as file:
            file.write(data)
            
    @classmethod
    def ReplaceEngineRootDirectory(cls, path, engineRoot):
        engineRootToken = "$ENGINE_ROOT$"
        engineRoot = str.replace(engineRoot, "\\", "/")
        with open(path, 'r',encoding='UTF-8') as file:
            data = file.read()
            data = data.replace(engineRootToken, engineRoot)
        with open(path, 'w',encoding='UTF-8') as file:
	        file.write(data)

if __name__ == "__main__":
    NewProject.Create()
