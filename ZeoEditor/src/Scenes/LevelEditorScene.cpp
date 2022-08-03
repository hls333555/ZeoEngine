#include "Scenes/LevelEditorScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Systems.h"
#include "Core/EditorManager.h"
#include "Editors/LevelEditor.h"

namespace ZeoEngine {

	LevelEditorScene::LevelEditorScene(const Ref<LevelEditor>& levelEditor)
		: m_LevelEditor(levelEditor)
	{
	}

	void LevelEditorScene::OnAttach()
	{
		RegisterSystem<ParticleUpdateSystem>(shared_from_this());
		RegisterSystem<PhysicsSystem>(shared_from_this());
		RegisterSystem<NativeScriptSystem>(shared_from_this());
	}

	void LevelEditorScene::OnEvent(Event& e)
	{
		// TODO:
		//m_NativeScriptSystem->OnEvent(e);
	}

	void LevelEditorScene::OnRuntimeStart()
	{
		for (const auto& system : GetSystems())
		{
			system->OnRuntimeStart();
		}
	}

	void LevelEditorScene::OnRuntimeStop()
	{
		for (const auto& system : GetSystems())
		{
			system->OnRuntimeStop();
		}
	}

	void LevelEditorScene::PostLoad()
	{
		m_Registry.view<ParticleSystemComponent>().each([](auto entity, auto& particleComp)
		{
			ParticleSystemInstance::Create(particleComp);
		});
		
		m_Registry.view<BoundsComponent>().each([this](auto e, auto& boundsComp)
		{
			Entity entity = { e, shared_from_this() };
			entity.UpdateBounds();
		});
	}

	Entity LevelEditorScene::GetSelectedEntity() const
	{
		return m_LevelEditor->GetContextEntity();
	}

}
