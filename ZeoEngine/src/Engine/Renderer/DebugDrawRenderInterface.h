#pragma once

#define DEBUG_DRAW_EXPLICIT_CONTEXT
#include <debug_draw.hpp>

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	struct SceneContext;
	class SceneRenderer;

	class DDRenderInterface : public dd::RenderInterface
	{
	public:
		void Init(const Ref<SceneContext>& sceneContext);
		static void Flush(const Ref<SceneContext>& sceneContext, float timeInMs);
		static void Shutdown(const Ref<SceneContext>& sceneContext);

		virtual void UpdateViewportSize(uint32_t width, uint32_t height) = 0;

		static Ref<DDRenderInterface> Create(const Ref<SceneRenderer>& sceneRenderer);
	};

}
