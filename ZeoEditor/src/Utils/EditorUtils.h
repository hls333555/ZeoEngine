#pragma once

#include "Core/EditorTypes.h"

namespace ZeoEngine {

	class EditorUtils
	{
	public:
		static const char* GetEditorName(EditorType editorType);
		static const char* GetPanelName(PanelType panelType);
	};
	
}
