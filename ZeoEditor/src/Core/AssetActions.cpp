#include "Core/AssetActions.h"

#include "Engine/GameFramework/ParticleSystem.h"
#include "Core/EditorManager.h"
#include "Editors/EditorBase.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	void SceneAssetActions::OpenAsset(const std::string& path)
	{
		EditorManager::Get().GetEditor(EditorType::MainEditor)->OpenScene(path);
	}

	void ParticleAssetActions::OpenAsset(const std::string& path)
	{
		auto particleAsset = ParticleLibrary::Get().LoadAsset(path);
		EditorBase* editor = EditorManager::Get().OpenEditor(EditorType::ParticleEditor);
		editor->GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([&](auto& particlePreviewComp)
		{
			particlePreviewComp.Template = particleAsset;
			ParticleSystemInstance::Create(particlePreviewComp);
		});
	}

}
