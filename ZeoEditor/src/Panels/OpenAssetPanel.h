#pragma once

#include "Panels/AssetBrowserPanelBase.h"

namespace ZeoEngine {

	class OpenAssetPanel : public AssetBrowserPanelBase
	{
	public:
		OpenAssetPanel(std::string panelName, const Weak<EditorBase>& contextEditor, AssetTypeID assetTypeID);

		virtual void OnAttach() override;

		AssetTypeID GetAssetTypeID() const { return m_AssetTypeID; }
		
	private:
		virtual bool WillDrawBottom() const override { return true; }
		virtual void DrawBottom() override;

		virtual bool ShouldDrawPath(const Ref<PathMetadata>& metadata) override;

		virtual void HandleRightColumnAssetOpen(const std::filesystem::path& path) override;

	private:
		AssetTypeID m_AssetTypeID;
	};

}
