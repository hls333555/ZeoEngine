#pragma once

#include "Reflection/ComponentInspector.h"

namespace ZeoEngine {

	class MaterialInspector : public ComponentInspector
	{
	public:
		using ComponentInspector::ComponentInspector;

		virtual I32 ProcessComponent() override;

	private:
		void DrawUniformData(const Ref<DynamicUniformDataBase>& uniform) const;
	};

}
