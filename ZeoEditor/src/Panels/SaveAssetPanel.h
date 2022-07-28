#pragma once

#include "Panels/OpenAssetPanel.h"

namespace ZeoEngine {

	class SaveAssetPanel : public OpenAssetPanel
	{
	public:
		using OpenAssetPanel::OpenAssetPanel;

	private:
		virtual void OnPanelOpen() override;

		virtual void OnPathSelected(const std::filesystem::path& path) override;

		virtual void DrawBottom() override;

		virtual void HandleRightColumnAssetOpen(const std::filesystem::path& path) override;

		void DrawReplaceDialog(const std::filesystem::path& path);

	private:
		char m_NameBuffer[MAX_PATH_SIZE] = "New";
		bool m_bHasKeyboardFocused = false;
		std::filesystem::path m_ToReplacePath;
	};

}
