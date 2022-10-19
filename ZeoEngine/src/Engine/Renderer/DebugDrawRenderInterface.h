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
		void Init(SceneContext* sceneContext);
		void Flush(float timeInMs) const;
		void Shutdown() const;

		virtual void UpdateViewportSize(U32 width, U32 height) = 0;

		static Ref<DDRenderInterface> Create(SceneRenderer* sceneRenderer);

	private:
		dd::ContextHandle m_Context = nullptr;
	};

}
