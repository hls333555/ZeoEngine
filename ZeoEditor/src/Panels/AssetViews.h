#pragma once

#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class EditorPreviewWorldBase;

	class AssetView
	{
	public:
		virtual ~AssetView() = default;

		virtual void Draw() {}
		virtual void DrawToolBar();
	};

	class TextureAssetView : public AssetView
	{
	public:
		explicit TextureAssetView(const Ref<EditorPreviewWorldBase>& world);

		virtual void Draw() override;
		virtual void DrawToolBar() override;

	private:
		Weak<EditorPreviewWorldBase> m_EditorWorld;
	};
	
}
