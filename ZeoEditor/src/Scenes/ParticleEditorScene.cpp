#include "Scenes/ParticleEditorScene.h"

#include "Engine/GameFramework/Systems.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	void ParticleEditorScene::OnUpdate(DeltaTime dt)
	{
		m_RenderSystem->OnUpdate(dt);
	}

	void ParticleEditorScene::OnRender(const EditorCamera& camera)
	{
		m_RenderSystem->OnRenderEditor(camera);
	}

}
