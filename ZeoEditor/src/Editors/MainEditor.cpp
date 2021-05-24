#include "Editors/MainEditor.h"

#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	void MainEditor::OnAttach()
	{
		EditorBase::OnAttach();

		m_PostSceneCreate.connect<&MainEditor::ClearSelectedEntity>(this);
	}

	AssetTypeId MainEditor::GetAssetTypeId() const
	{
		return AssetType<Scene>::Id();
	}

	void MainEditor::Serialize(const std::string& filePath)
	{
		SceneSerializer::Serialize(filePath, GetScene());
	}

	void MainEditor::Deserialize(const std::string& filePath)
	{
		SceneSerializer::Deserialize(filePath, GetScene());
	}

	void MainEditor::ClearSelectedEntity()
	{
		SetContextEntity({});
	}

}
