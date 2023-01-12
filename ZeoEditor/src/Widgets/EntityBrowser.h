#pragma once

#include "Widgets/TextFilter.h"

namespace ZeoEngine {

	struct EntityBrowser
	{
		TextFilter Filter;
		U32 LastFilteredCount = 0;

		EntityBrowser() = default;

		/**
		 * Draw the entity browsing widget.
		 *
		 * @param outEntityID - UUID of the selected entity
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @return True if the selection changed
		 */
		bool Draw(UUID& outEntityID, float rightPadding);
	};

}
