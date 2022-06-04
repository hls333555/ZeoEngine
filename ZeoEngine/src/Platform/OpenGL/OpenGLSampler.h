#pragma once

#include "Engine/Renderer/Sampler.h"

namespace ZeoEngine {

	class OpenGLSampler : public Sampler
	{
	public:
		OpenGLSampler(SamplerType type);
		virtual ~OpenGLSampler();

		virtual void Bind(U32 slot) const override;
		virtual void Unbind(U32 slot) const override;

	private:
		U32 m_RendererID = 0;
	};

}
