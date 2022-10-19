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

		void LoadAsset(const std::string& path, bool bForce);
		
		void SaveAsset(const std::string& path) const;
		void SaveAssetAs();
		void OnAssetSaveAs(const std::string& path);

		EditorCamera& GetEditorCamera() { return m_EditorCamera; }

		virtual Entity GetContextEntity() const override final { return m_ContextEntity; }
		void SetContextEntity(Entity entity);
		/** If bIsTeleport is true, the camera will trigger an immediately focus instead of interpolating several frames. */
		void FocusContextEntity(bool bIsTeleport = false);

		InspectorBase& GetInspector() const { return *m_Inspector; }
		virtual AssetView* GetAssetView() const { return nullptr; }

	private:
		virtual Ref<IAsset> LoadAssetImpl(const std::string& path, bool bForce) = 0;

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
