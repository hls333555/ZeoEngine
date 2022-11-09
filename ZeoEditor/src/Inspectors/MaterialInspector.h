#pragma once

#include "Inspectors/AssetInspector.h"

namespace ZeoEngine {

	struct DynamicUniformFieldBase;

	class MaterialInspector : public AssetInspector
	{
	public:
		using AssetInspector::AssetInspector;

	private:
		virtual void ProcessDraw(Entity entity) override;

	private:
		void DrawFieldWidget(const Ref<DynamicUniformFieldBase>& field) const;
	};

}
