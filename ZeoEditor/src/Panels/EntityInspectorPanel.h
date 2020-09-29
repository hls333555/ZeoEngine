#pragma once

#include "Panels/EditorPanel.h"

#include "Engine/GameFramework/Entity.h"
#include "Reflection/ReflectionHelper.h"

namespace ZeoEngine {

	class EntityInspectorPanel : public ScenePanel
	{
	public:
		using ScenePanel::ScenePanel;

	private:
		virtual void RenderPanel() override;

		void DrawInherentComponents(Entity entity);
		void DrawComponents(Entity entity);

		template<typename T>
		void ShowPropertyTooltip(T metaObj)
		{
			if (ImGui::IsItemHovered())
			{
				const char* tooltip = GetPropTooltip(metaObj);
				if (tooltip != "")
				{
					ImGui::SetTooltip(tooltip);
				}
			}
		}
		void ProcessType(entt::meta_type type, Entity entity);
		// NOTE: Do not pass entt::meta_handle around as it does not support copy
		void ProcessIntegralData(entt::meta_data data, entt::meta_any instance);
		void ProcessEnumData(entt::meta_data data, entt::meta_any instance);
		void ProcessOtherData(entt::meta_data data, entt::meta_any instance);

		void ProcessStringData(entt::meta_data data, entt::meta_any instance);
	};

}
