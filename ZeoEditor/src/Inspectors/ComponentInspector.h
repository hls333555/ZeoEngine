#pragma once

#include <map>
#include <deque>

#include "Inspectors/InspectorBase.h"
#include "Inspectors/DataParser.h"

namespace ZeoEngine {

	class DataWidget;

	class ComponentInspector : public InspectorBase
	{
	public:
		explicit ComponentInspector(U32 compId);
		
		virtual void Draw(Entity entity) override;

		U32 GetWillRemoveComponentId() const { return m_bWillRemove ? m_ComponentId : 0; }

	private:
		/** Iterate all datas on this component, reverse their order and categorize them. */
		void PreprocessComponent();
		void PreprocessData(entt::meta_data data);

		void DrawDataWidget(entt::meta_data data, entt::meta_any& compInstance);

	protected:
		U32 GetTableID() const { return m_TableID; }

	private:
		U32 m_ComponentId;
		bool m_bWillRemove = false;

		U32 m_TableID;

		/** Map from aggregated data id to DataWidget instance */
		std::unordered_map<U32, Ref<DataWidget>> m_DataWidgets;
		
		DataParser m_DataParser;

		/** Map from category to list of data ids in order */
		std::map <std::string, std::deque<U32>> m_PreprocessedDatas;
	};

}
