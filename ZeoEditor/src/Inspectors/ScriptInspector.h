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
		virtual void DrawExtraFieldWidgets(Entity entity, U32 backupItemID) override;
		void DrawScriptFieldWidget(const Ref<ScriptFieldInstance>& fieldInstance);
		void ClearFieldsAndWidgets();
		void CategorizeFields(Entity entity);

	private:
		/** Map from aggregated field ID to script field widget */
		std::unordered_map<U32, Scope<IFieldWidget>> m_ScriptFieldWidgets;
		/** Map from category to list of fields */
		std::map<std::string, std::vector<Ref<ScriptFieldInstance>>> m_CategorizedFields;
		std::string m_CurrentClassName;
	};
	
}
