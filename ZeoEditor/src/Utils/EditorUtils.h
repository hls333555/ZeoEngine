#pragma once

#include <magic_enum.hpp>

#include "Core/EditorTypes.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class EditorUtils
	{
	public:
		static const char* GetEditorName(EditorType editorType);
		static const char* GetPanelName(PanelType panelType);

	};
	
}
