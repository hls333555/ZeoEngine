#pragma once

#include "Widgets/TextFilter.h"

namespace ZeoEngine {

	struct ScriptClassBrowser
	{
		TextFilter Filter;
		U32 LastFilteredCount = 0;

		ScriptClassBrowser() = default;

		/**
		 * Draw the script class browsing widget.
		 *
		 * @param outClassName - class name of the selected script
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @return True if the selection changed
		 */
		bool Draw(std::string& outClassName, float rightPadding);
	};

}
