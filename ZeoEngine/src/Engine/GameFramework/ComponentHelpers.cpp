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

	IComponentHelper::IComponentHelper(IComponent* comp)
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

	LightComponentHelper::LightComponentHelper(IComponent* comp)
		: IComponentHelper(comp)
	{
		InitLight(comp);
	}

	void LightComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue)
	{
		auto& lightComp = GetOwnerEntity()->GetComponent<LightComponent>();
		if (dataId == ZDATA_ID(Type))
		{
			InitLight(&lightComp);
		}
	}

	void LightComponentHelper::InitLight(IComponent* comp)
	{
		auto* lightComp = dynamic_cast<LightComponent*>(comp);
		switch (lightComp->Type)
		{
			case LightComponent::LightType::DirectionalLight:	lightComp->Light = CreateRef<DirectionalLight>(); break;
			case LightComponent::LightType::PointLight:			lightComp->Light = CreateRef<PointLight>(); break;
			case LightComponent::LightType::SpotLight:			lightComp->Light = CreateRef<SpotLight>(); break;
			default: break;
		}
	}

}
