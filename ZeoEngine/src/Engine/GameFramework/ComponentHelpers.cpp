#include "ZEpch.h"
#include "Engine/GameFramework/ComponentHelpers.h"

#include "Engine/GameFramework/Components.h"
#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

	void ParticleSystemComponentHelper::OnComponentDestroy()
	{
		auto& particleComp = m_OwnerEntity.GetComponent<ParticleSystemComponent>();
		if (particleComp.Template)
		{
			particleComp.Template->RemoveParticleSystemInstance(particleComp.Instance);
		}
	}

	void ParticleSystemComponentHelper::OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue)
	{
		auto& particleComp = m_OwnerEntity.GetComponent<ParticleSystemComponent>();
		ParticleSystemInstance::Create(particleComp);
	}

	void ParticleSystemComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue)
	{
		auto& particleComp = m_OwnerEntity.GetComponent<ParticleSystemComponent>();
		if (dataId == ZDATA_ID(Template))
		{
			Asset<ParticleTemplate> oldTemplate = (*oldValue._Cast<Asset<ParticleTemplate>>());
			if (oldTemplate)
			{
				oldTemplate->RemoveParticleSystemInstance(particleComp.Instance);
			}
		}
		// Manually clear particle template selection
		if (!particleComp.Template)
		{
			particleComp.Instance.reset();
		}
		else
		{
			ParticleSystemInstance::Create(particleComp);
		}
	}

	void ParticleSystemPreviewComponentHelper::OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue)
	{
		auto& particlePreviewComp = m_OwnerEntity.GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.Template->ResimulateAllParticleSystemInstances();
	}

	void ParticleSystemPreviewComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue)
	{
		auto& particlePreviewComp = m_OwnerEntity.GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.Template->ResimulateAllParticleSystemInstances();
	}

}
