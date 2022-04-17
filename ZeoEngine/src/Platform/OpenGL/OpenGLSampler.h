#pragma once

#include "Engine/Renderer/Sampler.h"

namespace ZeoEngine {

	class OpenGLSampler : public Sampler
	{
	public:
		OpenGLSampler(SamplerType type);
		virtual ~OpenGLSampler();

		virtual void Bind(uint32_t slot) const override;
		virtual void Unbind(uint32_t slot) const override;

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Binding = 0;
	};

}
