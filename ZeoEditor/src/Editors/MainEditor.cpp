#include "Editors/MainEditor.h"

#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	void MainEditor::OnAttach()
	{
		EditorBase::OnAttach();

		m_PostSceneCreate.connect<&MainEditor::ClearSelectedEntity>(this);
	}

	void MainEditor::Serialize(const std::string& filePath)
	{
		SceneSerializer serializer(filePath, GetScene());
		serializer.Serialize();
	}

	void MainEditor::Deserialize(const std::string& filePath)
	{
		SceneSerializer serializer(filePath, GetScene());
		serializer.Deserialize();
	}

	void MainEditor::ClearSelectedEntity()
	{
		SetContextEntity({});
	}

}
