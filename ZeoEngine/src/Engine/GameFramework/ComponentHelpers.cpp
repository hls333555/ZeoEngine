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
		Ref<ParticleTemplate> newTemplate = particleComp.Template;
		if (dataId == ZE_DATA_ID(Template))
		{
			particleComp.Template = (*oldValue._Cast<Ref<ParticleTemplate>>());
		}
		// Manually clear particle template selection
		if (!newTemplate)
		{
			if (particleComp.Template)
			{
				particleComp.Template->RemoveParticleSystemInstance(particleComp.Instance);
				particleComp.Template = newTemplate; // Should be empty
				particleComp.Instance.reset();
			}
		}
		else
		{
			ParticleSystemInstance::Create(particleComp, newTemplate);
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
