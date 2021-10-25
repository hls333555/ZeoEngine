#include "ZEpch.h"
#include "Engine/GameFramework/ComponentHelpers.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

	struct IComponentHelper::Impl
	{
		Entity OwnerEntity;
	};

	IComponentHelper::IComponentHelper()
		: m_Impl(CreateScope<Impl>())
	{
	}

	IComponentHelper::~IComponentHelper() = default;

	Entity* IComponentHelper::GetOwnerEntity() const
	{
		return &m_Impl->OwnerEntity;
	}

	void IComponentHelper::SetOwnerEntity(Entity* entity)
	{
		m_Impl->OwnerEntity = *entity;
	}

	void ParticleSystemComponentHelper::OnComponentCopied()
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		ParticleSystemInstance::Create(particleComp);
	}

	void ParticleSystemComponentHelper::OnComponentDestroy()
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		if (particleComp.Template)
		{
			particleComp.Template->RemoveParticleSystemInstance(particleComp.Instance);
		}
	}

	void ParticleSystemComponentHelper::OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue)
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		ParticleSystemInstance::Create(particleComp);
	}

	void ParticleSystemComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue)
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		if (dataId == ZDATA_ID(Template))
		{
			AssetHandle<ParticleTemplateAsset> oldTemplate = (*oldValue._Cast<AssetHandle<ParticleTemplateAsset>>());
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
		auto& particlePreviewComp = GetOwnerEntity()->GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.Template->ResimulateAllParticleSystemInstances();
	}

	void ParticleSystemPreviewComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue)
	{
		auto& particlePreviewComp = GetOwnerEntity()->GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.Template->ResimulateAllParticleSystemInstances();
	}

}
