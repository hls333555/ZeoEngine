#pragma once

#include "Reflection/ComponentInspector.h"

namespace ZeoEngine {

	class MaterialInspector : public ComponentInspector
	{
	public:
		using ComponentInspector::ComponentInspector;

		virtual int32_t ProcessComponent() override;

	private:
		void DrawUniformData(const Scope<DynamicUniformDataBase>& uniform);
	};

}
