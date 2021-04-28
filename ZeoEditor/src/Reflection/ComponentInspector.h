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
		ComponentInspector(uint32_t compId, Entity entity);

		/** Returns this component id if it is about to remove or -1 otherwise. */
		int32_t ProcessComponent();
	private:
		/** Iterate all datas on this component, reverse their order and categorize them. */
		void PreprocessComponent();
		void PreprocessData(entt::meta_data data);

		void DrawDataWidget(entt::meta_data data, entt::meta_any& compInstance);

	private:
		uint32_t m_ComponentId;
		Entity m_OwnerEntity;

		DataParser m_DataParser;

		/** Map from aggregated data id to DataWidget instance */
		std::unordered_map<uint32_t, Ref<DataWidget>> m_DataWidgets;

		/** Map from category to list of data ids in order */
		std::map <std::string, std::deque<uint32_t>> m_PreprocessedDatas;
		bool m_bIsPreprocessedDatasDirty = true;
	};

}
