#pragma once

#include "Panels/AssetBrowserPanelBase.h"

namespace ZeoEngine {

	class ContentBrowserPanel : public AssetBrowserPanelBase
	{
	public:
		using AssetBrowserPanelBase::AssetBrowserPanelBase;

		virtual void OnAttach() override;

	private:
		void InitAssetTypeFilters();

		virtual void DrawTopBar() override;
		virtual bool IsAnyFilterActive() const override { return AssetBrowserPanelBase::IsAnyFilterActive() || m_bIsAnyTypeFilterActive; }
		virtual bool ShouldUpdateFilterCache() const override { return AssetBrowserPanelBase::ShouldUpdateFilterCache() || m_bIsTypeFilterChanged; }
		virtual bool PassFilter(const Ref<PathSpec>& spec) const override;
		virtual void ClearAllFilters() override;
		virtual void DrawWindowContextMenuImpl(float thumbnailWidth) override;
		virtual void DrawPathContextMenuItem_Save(const std::string& path, bool bIsAsset) override;
		virtual void DrawPathContextMenuItem_Asset(const std::string& path, const Ref<PathSpec>& spec) override;

		virtual void ProcessAssetDragging(const Ref<PathSpec>& spec, float thumbnailRounding) override;

		virtual void HandleRightColumnAssetOpen(const std::string& path) override;

	private:
		struct AssetTypeFilterSpec
		{
			AssetTypeId TypeId;
			const char* TypeName;
			bool bIsFilterActive = false;
		};
		std::vector<AssetTypeFilterSpec> m_AssetTypeFilters;
		bool m_bIsAnyTypeFilterActive = false;
		bool m_bIsTypeFilterChanged = false;
	};

}
