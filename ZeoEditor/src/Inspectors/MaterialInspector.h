#pragma once

#include "Inspectors/AssetInspector.h"

namespace ZeoEngine {

	struct DynamicUniformFieldBase;
	class DynamicUniformFieldWidgetBase;

	class MaterialInspector : public AssetInspector
	{
	public:
		MaterialInspector(AssetPreviewWorldBase* world, U32 compID);
		~MaterialInspector();

	private:
		virtual void ProcessDraw(Entity entity) override;

		void DrawFieldWidget(const Ref<DynamicUniformFieldBase>& field);
		void DrawDynamicUniformFieldWidget(const Ref<DynamicUniformFieldBase>& field);
		void ClearFieldWidgets();

	private:
		/** Map from aggregated field ID to dynamic uniform field widget */
		std::unordered_map<U32, Scope<DynamicUniformFieldWidgetBase>> m_DynamicUniformFieldWidgets;
		AssetHandle m_CurrentMaterialAsset = 0;
	};

}
