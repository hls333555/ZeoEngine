#pragma once

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
		explicit TextureAssetView(const EditorPreviewWorldBase* world);

		virtual void Draw() override;
		virtual void DrawToolBar() override;

	private:
		const EditorPreviewWorldBase* m_EditorWorld;
	};
	
}
