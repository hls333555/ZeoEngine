#pragma once

#include "Engine/GameFramework/World.h"

#include "Engine/Renderer/EditorCamera.h"

namespace ZeoEngine {

	class InspectorBase;
	class AssetView;

	class EditorPreviewWorldBase : public WorldBase
	{
	public:
		explicit EditorPreviewWorldBase(std::string worldName);
		virtual ~EditorPreviewWorldBase();

		virtual void OnAttach() override;
		virtual void OnUpdate(DeltaTime dt) override;

		virtual void LoadAsset(const std::string& path) = 0;
		void SaveAsset(const std::string& path) const;
		void SaveAssetAs();
		/** You should force load asset to replace current preview one here. */
		virtual void OnAssetSaveAs(const std::string& path) = 0;

		EditorCamera& GetEditorCamera() { return m_EditorCamera; }

		Entity GetContextEntity() const { return m_ContextEntity; }
		void SetContextEntity(Entity entity);
		/** If bIsTeleport is true, the camera will trigger an immediately focus instead of interpolating several frames. */
		void FocusContextEntity(bool bIsTeleport = false);

		InspectorBase& GetInspector() const { return *m_Inspector; }
		virtual AssetView* GetAssetView() const { return nullptr; }

	private:
		virtual Scope<InspectorBase> CreateInspector() = 0;

	public:
		entt::sink<entt::sigh<void(Entity, Entity)>> m_OnContextEntityChanged{ m_OnContextEntityChangedDel };

	private:
		EditorCamera m_EditorCamera = EditorCamera(50.625f, 1.778f, 0.1f, 1000.0f);

		Entity m_ContextEntity;
		entt::sigh<void(Entity, Entity)> m_OnContextEntityChangedDel;

		Scope<InspectorBase> m_Inspector;
	};

}
