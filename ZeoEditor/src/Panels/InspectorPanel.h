#pragma once

#include <stack>

#include "Engine/GameFramework/Entity.h"
#include "Panels/PanelBase.h"

namespace ZeoEngine {

	class EditorPreviewWorldBase;
	class AssetViewPanel;

	struct InspectHistoryData
	{
		EditorPreviewWorldBase* World = nullptr;
		Entity LevelSelectedEntity;
		std::string AssetPath;

		InspectHistoryData(EditorPreviewWorldBase* world, Entity levelSelectedEntity)
			: World(world), LevelSelectedEntity(levelSelectedEntity) {}
		InspectHistoryData(EditorPreviewWorldBase* world, std::string assetPath)
			: World(world), AssetPath(std::move(assetPath)) {}
	};

	class InspectorPanel : public PanelBase
	{
	public:
		explicit InspectorPanel(std::string panelName);

		EditorPreviewWorldBase* GetEditorWorld() const { return m_EditorWorld; }
		AssetViewPanel& GetAssetViewPanel() const { return *m_AssetViewPanel; }

		void UpdateWorld(EditorPreviewWorldBase* world, bool bIncludeAssetViewPanel);
		void ToggleAssetView(bool bShow) const;

		void AddInspectHistory(InspectHistoryData data) { m_InspectHistory.push(std::move(data)); }
		void ClearInspectHistory() { m_InspectHistory = {}; }
		void InspectLast();
		bool IsLastInspectHistoryValid() const { return !m_InspectHistory.empty() && (m_InspectHistory.top().LevelSelectedEntity.IsValid() || !m_InspectHistory.top().AssetPath.empty()); }
		/** You must call IsLastInspectHistoryValid() before calling this to make sure there is a valid name to display. */
		std::string GetLastInspectHistoryDisplayName() const;

	private:
		virtual void ProcessUpdate(DeltaTime dt) override;
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e) override;

		void RenderDetailsPanel(Entity selectedEntity);
		void BuildDockspaceLayout(ImGuiID dockspaceID);

	private:
		EditorPreviewWorldBase* m_EditorWorld = nullptr;

		Scope<AssetViewPanel> m_AssetViewPanel;
		/** Flag for details panel to update once for last selected entity which fixes a issue where field caches cannot be applied when an empty entity is selected */
		bool m_bShouldDrawWhenNoEntitySelected = true;

		std::stack<InspectHistoryData> m_InspectHistory;

	};

}
