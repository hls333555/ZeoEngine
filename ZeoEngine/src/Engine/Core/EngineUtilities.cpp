#include "ZEpch.h"
#include "Engine/Core/EngineUtilities.h"

#include "Engine/Renderer/OrthographicCamera.h"

namespace ZeoEngine {

	glm::vec2 ProjectScreenToWorld2D(const glm::vec2& screenPosition, ImGuiWindow* gameViewWindow, OrthographicCamera* camera)
	{
		ZE_CORE_ASSERT(gameViewWindow && camera);

		const auto& cameraBounds = camera->GetCameraBounds();
		// We should refer to Game View window's inner rect instead of Game View window
		float worldX = (screenPosition.x - gameViewWindow->InnerRect.Min.x) / gameViewWindow->InnerRect.GetSize().x *
			// Left bound is a negative value
			(-cameraBounds.Left + cameraBounds.Right) +
			cameraBounds.Left +
			// Corrected by camera position
			camera->GetPosition().x;
		float worldY = (screenPosition.y - gameViewWindow->InnerRect.Min.y) / gameViewWindow->InnerRect.GetSize().y *
			// Window position y increases from top to bottom while OpenGL position y increases from bottom to top,
			// level origin (0, 0) is at the center of Game View window
			(cameraBounds.Bottom - cameraBounds.Top) +
			cameraBounds.Top +
			// Corrected by camera position
			camera->GetPosition().y;

		return { worldX, worldY };
	}

	// This function is essentially an inverse process of the above one
	glm::vec2 ProjectWorldToScreen2D(const glm::vec2& worldPosition, ImGuiWindow* gameViewWindow, OrthographicCamera* camera)
	{
		ZE_CORE_ASSERT(gameViewWindow && camera);

		const auto& cameraBounds = camera->GetCameraBounds();
		float screenX = (worldPosition.x - camera->GetPosition().x - cameraBounds.Left) /
			(-cameraBounds.Left + cameraBounds.Right) *
			gameViewWindow->InnerRect.GetSize().x +
			gameViewWindow->InnerRect.Min.x;
		float screenY = (worldPosition.y - camera->GetPosition().y - cameraBounds.Top) /
			(cameraBounds.Bottom - cameraBounds.Top) *
			gameViewWindow->InnerRect.GetSize().y +
			gameViewWindow->InnerRect.Min.y;

		return { screenX, screenY };
	}

}
