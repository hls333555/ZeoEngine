#include "Scenes/MaterialEditorScene.h"

#include "Engine/GameFramework/Systems.h"

namespace ZeoEngine {

	MaterialEditorScene::MaterialEditorScene()
	{
		m_MaterialPreviewRenderSystem = CreateScope<MaterialPreviewRenderSystem>(this);
		m_MaterialPreviewRenderSystem->OnCreate();
	}

	void MaterialEditorScene::OnRender(const EditorCamera& camera)
	{
		m_MaterialPreviewRenderSystem->OnRender(camera);
	}

}
