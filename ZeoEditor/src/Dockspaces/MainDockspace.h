#pragma once

#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	class MainDockspace : public EditorDockspace
	{
	public:
		using EditorDockspace::EditorDockspace;

		virtual void OnAttach() override;

		virtual void CreateNewScene() override;

		Entity& GetSeletedEntity() { return m_ContextEntity; }

	private:
		virtual AssetType GetAssetType() const override { return AssetType::Scene; }
		virtual void Serialize(const std::string& filePath) override;
		virtual void Deserialize(const std::string& filePath) override;

		virtual void BuildDockWindows(ImGuiID dockspaceID) override;

	};

}
