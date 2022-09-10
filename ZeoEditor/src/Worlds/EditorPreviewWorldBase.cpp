#include "Worlds/EditorPreviewWorldBase.h"

#include "Core/Editor.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Profile/BenchmarkTimer.h"
#include "Inspectors/InspectorBase.h"
#include "Panels/SaveAssetPanel.h"

namespace ZeoEngine {

	EditorPreviewWorldBase::EditorPreviewWorldBase(std::string worldName)
		: WorldBase(std::move(worldName))
	{
	}

	EditorPreviewWorldBase::~EditorPreviewWorldBase() = default;

	void EditorPreviewWorldBase::OnAttach()
	{
		WorldBase::OnAttach();

		m_Inspector = CreateInspector();
	}

	void EditorPreviewWorldBase::OnUpdate(DeltaTime dt)
	{
		m_EditorCamera.OnUpdate(dt);

		WorldBase::OnUpdate(dt);
	}

	void EditorPreviewWorldBase::SaveAsset(const std::string& path) const
	{
		Timer timer;
		if (AssetManager::Get().SaveAsset(path, GetAsset()))
		{
			ZE_CORE_WARN("Saving \"{0}\" took {1} ms", path, timer.ElapsedMillis());
		}
	}

	void EditorPreviewWorldBase::SaveAssetAs()
	{
		auto saveAssetPanel = g_Editor->GetPanel<SaveAssetPanel>(SAVE_ASSET);
		if (!saveAssetPanel)
		{
			saveAssetPanel = g_Editor->CreatePanel<SaveAssetPanel>(SAVE_ASSET, GetAsset()->GetTypeID(), SharedFromBase<EditorPreviewWorldBase>());
		}
		else
		{
			saveAssetPanel->Toggle(true);
		}
	}

	void EditorPreviewWorldBase::SetContextEntity(Entity entity)
	{
		if (entity == m_ContextEntity) return;

		const Entity lastEntity = m_ContextEntity;
		m_ContextEntity = entity;
		m_OnContextEntityChangedDel.publish(entity, lastEntity);
	}

	void EditorPreviewWorldBase::FocusContextEntity(bool bIsTeleport)
	{
		m_EditorCamera.StartFocusEntity(GetContextEntity(), bIsTeleport);
	}

}
