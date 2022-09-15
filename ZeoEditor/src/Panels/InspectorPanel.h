#pragma once

#include <stack>

#include "Engine/GameFramework/Entity.h"
#include "Panels/PanelBase.h"

namespace ZeoEngine {

	class EditorPreviewWorldBase;
	class AssetViewPanel;

	struct InspectHistoryData
	{
		Weak<EditorPreviewWorldBase> World;
		Entity LevelSelectedEntity;
		std::string AssetPath;

		InspectHistoryData(const Ref<EditorPreviewWorldBase>& world, Entity levelSelectedEntity)
			: World(world), LevelSelectedEntity(levelSelectedEntity) {}
		InspectHistoryData(const Ref<EditorPreviewWorldBase>& world, const std::string& assetPath)
			: World(world), AssetPath(assetPath) {}
	};

	class InspectorPanel : public PanelBase
	{
	public:
		explicit InspectorPanel(std::string panelName);

		Ref<EditorPreviewWorldBase> GetEditorWorld() const { return m_EditorWorld.lock(); }
		Ref<AssetViewPanel> GetAssetViewPanel() const { return m_AssetViewPanel; }

		void UpdateWorld(const Ref<EditorPreviewWorldBase>& world, bool bIncludeAssetViewPanel);
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

		void BuildDockspaceLayout(ImGuiID dockspaceID);

	private:
		Weak<EditorPreviewWorldBase> m_EditorWorld;

		Ref<AssetViewPanel> m_AssetViewPanel;

		std::stack<InspectHistoryData> m_InspectHistory;

	};

}
