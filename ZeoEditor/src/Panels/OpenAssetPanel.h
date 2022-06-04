#pragma once

#include "Panels/AssetBrowserPanelBase.h"

namespace ZeoEngine {

	class OpenAssetPanel : public AssetBrowserPanelBase
	{
	public:
		OpenAssetPanel(const char* panelName, const Weak<EditorBase>& contextEditor, AssetTypeId assetTypeId);

		virtual void OnAttach() override;

		AssetTypeId GetAssetTypeId() const { return m_AssetTypeId; }
		
	private:
		virtual bool WillDrawBottom() const override { return true; }
		virtual void DrawBottom() override;

		virtual bool ShouldDrawPath(const Ref<PathSpec>& spec) override;

		virtual void HandleRightColumnAssetOpen(const std::string& path) override;

	private:
		AssetTypeId m_AssetTypeId;
	};

}
