#pragma once

#include "Inspectors/AssetInspector.h"

namespace ZeoEngine {

	struct DynamicUniformDataBase;

	class MaterialInspector : public AssetInspector
	{
	public:
		using AssetInspector::AssetInspector;

	private:
		virtual void ProcessDraw(Entity entity) override;

	private:
		void DrawUniformData(const Ref<DynamicUniformDataBase>& data) const;
	};

}
