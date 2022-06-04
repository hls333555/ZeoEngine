#pragma once

#include <map>
#include <deque>

#include "Engine/GameFramework/Entity.h"
#include "Reflection/DataParser.h"

namespace ZeoEngine {

	class DataWidget;

	class ComponentInspector
	{
	public:
		ComponentInspector(U32 compId, Entity entity);

		/** Returns this component id if it is about to remove or -1 otherwise. */
		virtual I32 ProcessComponent();
	private:
		/** Iterate all datas on this component, reverse their order and categorize them. */
		void PreprocessComponent();
		void PreprocessData(entt::meta_data data);

		void DrawDataWidget(entt::meta_data data, entt::meta_any& compInstance);

	protected:
		Entity GetOwnerEntity() const { return m_OwnerEntity; }
		U32 GetTableID() const { return m_TableID; }

	private:
		U32 m_ComponentId;
		Entity m_OwnerEntity;

		U32 m_TableID;

		/** Map from aggregated data id to DataWidget instance */
		std::unordered_map<U32, Ref<DataWidget>> m_DataWidgets;
		
		DataParser m_DataParser;

		/** Map from category to list of data ids in order */
		std::map <std::string, std::deque<U32>> m_PreprocessedDatas;
		bool m_bIsPreprocessedDatasDirty = true;
	};

}
