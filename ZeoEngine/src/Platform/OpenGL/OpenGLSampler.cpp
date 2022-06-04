#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLSampler.h"

#include <glad/glad.h>

namespace ZeoEngine {

	OpenGLSampler::OpenGLSampler(SamplerType type)
	{
		glCreateSamplers(1, &m_RendererID);

		GLint filter, wrap;
		switch (type)
		{
			case SamplerType::PointClamp:
			case SamplerType::ShadowDepth:
				filter = GL_NEAREST;
				wrap = GL_CLAMP_TO_EDGE;
				break;
			case SamplerType::PointRepeat:
				filter = GL_NEAREST;
				wrap = GL_REPEAT;
				break;
			case SamplerType::BilinearClamp:
				filter = GL_LINEAR;
				wrap = GL_CLAMP_TO_EDGE;
				break;
			case SamplerType::ShadowPCF:
				filter = GL_LINEAR;
				wrap = GL_CLAMP_TO_EDGE;
				glSamplerParameteri(m_RendererID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				glSamplerParameteri(m_RendererID, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
				break;
		}
		glSamplerParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, filter);
		glSamplerParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, filter);
		glSamplerParameteri(m_RendererID, GL_TEXTURE_WRAP_R, wrap);
		glSamplerParameteri(m_RendererID, GL_TEXTURE_WRAP_S, wrap);
		glSamplerParameteri(m_RendererID, GL_TEXTURE_WRAP_T, wrap);
	}

	OpenGLSampler::~OpenGLSampler()
	{
		glDeleteSamplers(1, &m_RendererID);
	}

	void OpenGLSampler::Bind(uint32_t slot) const
	{
		glBindSampler(slot, m_RendererID);
	}

	void OpenGLSampler::Unbind(uint32_t slot) const
	{
		glBindSampler(slot, 0);
	}

}
