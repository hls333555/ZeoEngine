#include "ZEpch.h"
#include "Engine/Renderer/Sampler.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLSampler.h"

namespace ZeoEngine {

	Ref<Sampler> Sampler::Create(SamplerType type)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	ZE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return CreateRef<OpenGLSampler>(type);
		}

		ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::unordered_map<SamplerType, Ref<Sampler>> SamplerLibrary::m_Samplers;

	Ref<Sampler> SamplerLibrary::GetOrAddSampler(SamplerType type)
	{
		if (type == SamplerType::None) return {};

		auto it = m_Samplers.find(type);
		if (it != m_Samplers.end())
		{
			return it->second;
		}

		auto sampler = Sampler::Create(type);
		m_Samplers[type] = sampler;
		return sampler;
	}

}
