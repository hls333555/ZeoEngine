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

		virtual SamplerType GetType() const override { return m_Type; }

	private:
		U32 m_RendererID = 0;
		SamplerType m_Type;
	};

}
