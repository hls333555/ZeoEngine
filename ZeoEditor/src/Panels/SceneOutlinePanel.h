#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Log.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class SceneOutlinePanel
	{
	public:
		SceneOutlinePanel() = default;
		SceneOutlinePanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context);

		void OnImGuiRender();

	private:
		void DrawEntityNode(Entity entity);

	private:
		Ref<Scene> m_Context;
		Entity m_SelectedEntity;
	};

}
