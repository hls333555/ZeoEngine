#pragma once

#include <map>
#include <optional>

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class DataInspectorPanel;
	class DataWidget;

	class DataInspector
	{
	public:
		DataInspector(DataInspectorPanel* context);

		void ProcessComponent(entt::meta_type compType, Entity entity);
		/** Iterate all datas on this component, reverse their order and categorize them. */
		void PreprocessComponent(entt::meta_type compType);

		void OnDrawComponentsComplete();
		void OnSelectedEntityChanged();

	private:
		void PreprocessData(entt::meta_type compType, entt::meta_data data);

		// NOTE: Do not pass entt::meta_handle around as it does not support copy
		bool ShouldHideData(entt::meta_data data, const entt::meta_any& instance);
		void ParseHideCondition(entt::meta_data data, const entt::meta_any& instance, const std::string& hideConditionStr, const char* token, std::optional<bool>& result);

		void MarkCachesDirty();

		void DrawDataWidget(entt::meta_data data, const entt::meta_any& compInstance);

	private:
		DataInspectorPanel* m_Context;

		/** Map from aggregated data id to DataWidget instance */
		std::unordered_map<uint32_t, Ref<DataWidget>> m_DataWidgets;

		/** Map from category to list of data ids of component in order */
		using CategorizedDatas = std::map<std::string, std::list<uint32_t>>;
		/** Map from component id to all its data, used to draw ordered registered datas in DataInspectorPanel */
		std::unordered_map<uint32_t, CategorizedDatas> m_PreprocessedDatas;
		bool m_bIsPreprocessedDatasDirty = true;
	};

}
