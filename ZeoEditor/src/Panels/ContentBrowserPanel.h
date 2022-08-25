#pragma once

#include "Panels/AssetBrowserPanelBase.h"

#include "Engine/Events/ApplicationEvent.h"

namespace ZeoEngine {

	class ContentBrowserPanel : public AssetBrowserPanelBase
	{
	public:
		using AssetBrowserPanelBase::AssetBrowserPanelBase;

		virtual void OnAttach() override;

	private:
		virtual void ProcessEvent(Event& e) override;

		void InitAssetTypeFilters();

		virtual void DrawTopBar() override;
		virtual bool IsAnyFilterActive() const override { return AssetBrowserPanelBase::IsAnyFilterActive() || m_bIsAnyTypeFilterActive; }
		virtual bool ShouldUpdateFilterCache() const override { return AssetBrowserPanelBase::ShouldUpdateFilterCache() || m_bIsTypeFilterChanged; }
		virtual bool PassFilter(const Ref<PathMetadata>& metadata) const override;
		virtual void ClearAllFilters() override;
		virtual void DrawWindowContextMenuImpl(float thumbnailWidth) override;
		virtual void DrawPathContextMenuItem_Save(const std::string& path, bool bIsAsset) override;
		virtual void DrawPathContextMenuItem_Asset(const std::string& path, const Ref<AssetMetadata>& metadata) override;

		bool OnFileDropped(WindowFileDroppedEvent& e);
		void ImportAsset(const std::string& path);
		virtual void ProcessAssetDragging(const Ref<PathMetadata>& metadata, float thumbnailRounding) override;

		virtual void HandleRightColumnAssetOpen(const std::string& path) override;

	private:
		struct AssetTypeFilterInfo
		{
			AssetTypeID TypeID;
			const char* TypeName;
			bool bIsFilterActive = false;
		};
		std::vector<AssetTypeFilterInfo> m_AssetTypeInfos;
		bool m_bIsAnyTypeFilterActive = false;
		bool m_bIsTypeFilterChanged = false;
	};

}
