#pragma once

#include "Panels/EditorPanel.h"

#include "Engine/GameFramework/Entity.h"
#include "Reflection/RegistrationHelper.h"

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
		void ProcessType(entt::meta_type type);
		void ProcessIntegralData(entt::meta_data data);
		void ProcessEnumData(entt::meta_data data);
		void ProcessOtherData(entt::meta_data data);

	};

}
