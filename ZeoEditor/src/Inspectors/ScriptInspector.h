#pragma once

#include "Inspectors/ComponentInspector.h"

namespace ZeoEngine {

	class ScriptFieldInstance;
	class IFieldWidget;

	class ScriptInspector : public ComponentInspector
	{
	public:
		explicit ScriptInspector(U32 compID);
		~ScriptInspector();

	private:
		virtual void DrawExtraFieldWidgets(Entity entity) override;
		void DrawScriptFieldWidget(const Ref<ScriptFieldInstance>& fieldInstance);
		void ClearScriptFieldWidgets();

	private:
		/** Map from aggregated field ID to script field widget */
		std::unordered_map<U32, Scope<IFieldWidget>> m_ScriptFieldWidgets;
	};
	
}
