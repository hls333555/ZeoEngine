#pragma once

#include "Widgets/TextFilter.h"
#include "Inspectors/InspectorBase.h"

namespace ZeoEngine {

	class EditorPreviewWorldBase;
	class ComponentInspector;

	class EntityInspector : public InspectorBase
	{
	public:
		explicit EntityInspector(EditorPreviewWorldBase* world);

		virtual void Draw(Entity entity) override;

	private:
		void DrawInternal(Entity entity);
		void CategorizeComponents();
		void DrawAddComponentButton(Entity entity);

		void OnSelectedEntityChanged(Entity entity, Entity lastEntity);
		void RebuildComponentInspectors(Entity entity);

	private:
		std::vector<Scope<ComponentInspector>> m_ComponentInspectors;

		TextFilter m_Filter;
		bool m_bHasComponentFilterKeyboardFocused = false;

		/** Map from category to list of component IDs, used to draw categorized components in AddComponent popup */
		std::map<std::string, std::vector<U32>> m_CategorizedComponents;

		Entity m_LastEntity;
		bool m_bShouldRebuildComponentInspectors = false;
	};
	
}
