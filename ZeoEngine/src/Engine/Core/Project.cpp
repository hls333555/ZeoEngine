#include "ZEpch.h"
#include "Engine/Core/Project.h"

#include "Engine/Core/Serializer.h"
#include "Engine/Utils/FileSystemUtils.h"

namespace ZeoEngine {

	std::string Project::GetProjectDirectory() const
	{
		return FileSystemUtils::GetParentPath(m_ProjectFilePath);
	}

	Ref<Project> Project::New()
	{
		s_ActiveProject = CreateRef<Project>();
		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::string& path)
	{
		auto project = CreateRef<Project>();
		if (ProjectSerializer::Deserialize(path, *project))
		{
			s_ActiveProject = std::move(project);
			s_ActiveProject->m_ProjectFilePath = path;
			s_OnProjectLoadedDel.publish();
			return s_ActiveProject;
		}

		return nullptr;
	}

	void Project::Unload()
	{
		s_ActiveProject.reset();
		s_OnProjectUnloadedDel.publish();
	}

	void Project::Save()
	{
		ProjectSerializer::Serialize(s_ActiveProject->m_ProjectFilePath, *s_ActiveProject);
	}

}
