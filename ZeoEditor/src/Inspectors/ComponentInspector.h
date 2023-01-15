#pragma once

#include <map>
#include <deque>

#include "Inspectors/InspectorBase.h"

namespace ZeoEngine {

	class IFieldWidget;

	class ComponentInspector : public InspectorBase
	{
	public:
		explicit ComponentInspector(U32 compID);
		~ComponentInspector();

		U32 GetTableID() const { return m_TableID; }
		void SetTableID(U32 tableID) { m_TableID = tableID; }
		
		virtual void Draw(Entity entity) override;

		U32 GetWillRemoveComponentID() const { return m_bWillRemove ? m_ComponentID : 0; }

	private:
		/** Iterate all datas on this component, reverse their order and categorize them. */
		void PreprocessComponent();
		void PreprocessField(entt::meta_data data);
		bool ShouldHideField(entt::meta_data data, entt::meta_any& instance) const;

		void DrawFieldWidget(entt::meta_data data, Entity entity);
		virtual void DrawExtraFieldWidgets(Entity entity, U32 backupItemID) {}

	private:
		U32 m_ComponentID;
		bool m_bWillRemove = false;

		/** ImGui table ID used for column separator syncing */
		U32 m_TableID;

		/** Map from field ID to field widget */
		std::unordered_map<U32, Scope<IFieldWidget>> m_FieldWidgets;

		/** Map from category to list of field IDs in order */
		std::map <std::string, std::deque<U32>> m_PreprocessedFields;
	};

}
