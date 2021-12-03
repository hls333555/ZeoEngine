#pragma once

#include <debug_draw.hpp>

namespace ZeoEngine {

	class DDRenderInterface : public dd::RenderInterface
	{
	public:
		static void Init(const Scope<DDRenderInterface>& ddri);
		static void Flush(float timeInMs);
		static void Shutdown();

		virtual void Init() = 0;
		virtual void UpdateViewportSize(uint32_t width, uint32_t height) = 0;

		static Scope<DDRenderInterface> Create();
	};

}
