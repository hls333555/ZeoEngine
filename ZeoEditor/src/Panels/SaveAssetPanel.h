#pragma once

#include "Panels/OpenAssetPanel.h"

namespace ZeoEngine {

	class SaveAssetPanel : public OpenAssetPanel
	{
	public:
		OpenAssetPanel::OpenAssetPanel;

	private:
		virtual void OnPanelOpen() override;

		virtual void OnPathSelected(const std::string& path) override;

		virtual void DrawBottom() override;

		virtual void HandleRightColumnAssetOpen(const std::string& path) override;

		void DrawReplaceDialog(const std::string& path);

	private:
		char m_NameBuffer[GetMaxPathSize()] = "New";
		bool m_bHasKeyboardFocused = false;
		std::string m_ToReplacePath;
	};

}
