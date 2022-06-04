#pragma once

#include "Engine/Core/Core.h"

namespace ZeoEngine {

	enum class SamplerType
	{
		None = 0,

		BilinearRepeat,
		PointClamp,
		PointRepeat,
		BilinearClamp,
		ShadowDepth, // PointClamp
		ShadowPCF,
	};

	class Sampler
	{
		friend class SamplerLibrary;

	protected:
		virtual ~Sampler() = default;

	public:
		virtual void Bind(uint32_t slot) const = 0;
		virtual void Unbind(uint32_t slot) const = 0;

	private:
		static Ref<Sampler> Create(SamplerType type);
	};

	class SamplerLibrary
	{
	public:
		static Ref<Sampler> GetOrAddSampler(SamplerType type);
	private:
		static std::unordered_map<SamplerType, Ref<Sampler>> m_Samplers;
	};

}
