#include "Scenes/LevelPreviewScene.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Systems.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	void LevelPreviewScene::OnAttach(const Ref<WorldBase>& world)
	{
		RegisterSystem<ParticleUpdateSystem>(world);
		RegisterSystem<PhysicsSystem>(world);
		RegisterSystem<ScriptSystem>(world);
		RegisterSystem<NativeScriptSystem>(world);
	}

	void LevelPreviewScene::OnEvent(Event& e)
	{
		// TODO:
		//m_NativeScriptSystem->OnEvent(e);
	}

	void LevelPreviewScene::OnRuntimeStart() const
	{
		for (const auto& system : GetSystems())
		{
			system->OnRuntimeStart();
		}
	}

	void LevelPreviewScene::OnRuntimeStop() const
	{
		for (const auto& system : GetSystems())
		{
			system->OnRuntimeStop();
		}
	}

	void LevelPreviewScene::PostLoad()
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

}
