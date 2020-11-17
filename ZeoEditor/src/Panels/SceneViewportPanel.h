#pragma once

#include "Panels/EditorPanel.h"

#include <glm/glm.hpp>

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class SceneViewportPanel : public ScenePanel
	{
	public:
		using ScenePanel::ScenePanel;
		virtual void OnAttach() override;

		virtual void OnUpdate(DeltaTime dt) override;

		void CreatePreviewCamera(bool bIsFromOpenScene = false);

		void Snapshot(const std::string& fileName);

	protected:
		virtual void RenderPanel() override;

	private:
		void OnViewportResize(const glm::vec2& size);

		void InitCameraViewportSize(entt::registry& registry, entt::entity entity);

	private:
		Entity m_PreviewCamera;
		glm::vec2 m_LastViewportSize{ 0.0f };

	};

}
