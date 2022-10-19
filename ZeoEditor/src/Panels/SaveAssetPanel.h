#pragma once

#include "Panels/OpenAssetPanel.h"

namespace ZeoEngine {

	class EditorPreviewWorldBase;

	class SaveAssetPanel : public OpenAssetPanel
	{
	public:
		SaveAssetPanel(std::string panelName, AssetTypeID assetTypeID, EditorPreviewWorldBase* world);

	private:
		virtual void OnPanelOpen() override;

		virtual void OnPathSelected(const std::string& path) override;

		virtual void DrawBottom() override;

		virtual void HandleRightColumnAssetOpen(const std::string& path) override;

		void DrawReplaceDialog();
		void DrawReplaceErrorDialog();

		void SaveAndClose(const std::string& path);

	private:
		EditorPreviewWorldBase* m_World = nullptr;

		char m_NameBuffer[MAX_PATH_SIZE];
		bool m_bHasKeyboardFocused = false;
		std::string m_ToReplacePath;
		bool m_bReplaceError = false;
	};

}
