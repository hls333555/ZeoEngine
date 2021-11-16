#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class MaterialPreviewRenderSystem;

	class MaterialEditorScene : public Scene
	{
	public:
		MaterialEditorScene();

		virtual void OnRender(const EditorCamera& camera) override;

	private:
		Scope<MaterialPreviewRenderSystem> m_MaterialPreviewRenderSystem;
	};

}
