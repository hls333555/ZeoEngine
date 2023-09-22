#pragma once

#include "Widgets/TextFilter.h"

namespace ZeoEngine {

	struct CollisionLayerBrowser
	{
		TextFilter Filter;
		U32 LastFilteredCount = 0;

		CollisionLayerBrowser() = default;

		/**
		 * Draw the collision layer browsing widget.
		 *
		 * @param outCollisionLayer - selected collision layer
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @return True if the selection changed
		 */
		bool Draw(U32& outCollisionLayer, float rightPadding);
	};

	struct CollisionGroupBrowser
	{
		TextFilter Filter;
		U32 LastFilteredCount = 0;

		CollisionGroupBrowser() = default;

		/**
		 * Draw the collision group browsing widget.
		 *
		 * @param outCollisionGroup - selected collision group
		 * @param rightPadding - How much space to remain after drawing the combobox widget
		 * @return True if the selection changed
		 */
		bool Draw(U32& outCollisionGroup, float rightPadding);
	};

}
