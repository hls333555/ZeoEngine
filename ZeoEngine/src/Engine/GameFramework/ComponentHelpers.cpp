#include "ZEpch.h"
#include "Engine/GameFramework/ComponentHelpers.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

	struct IComponentHelper::Impl
	{
		Entity OwnerEntity;

		Impl(Entity entity)
			: OwnerEntity(entity) {}
	};

	IComponentHelper::IComponentHelper(Entity* entity)
		: m_Impl(CreateScope<Impl>(*entity))
	{
	}

	IComponentHelper::~IComponentHelper() = default;

	Entity* IComponentHelper::GetOwnerEntity() const
	{
		return &m_Impl->OwnerEntity;
	}

	void ParticleSystemComponentHelper::OnComponentCopied(IComponent* otherComp)
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

	void LightComponentHelper::OnComponentAdded()
	{
		GetOwnerEntity()->AddComponent<BillboardComponent>();
		InitLight();
	}

	void LightComponentHelper::OnComponentCopied(IComponent* otherComp)
	{
		// Perform the deep copy of smart pointer
		auto* otherLightComp = static_cast<LightComponent*>(otherComp);
		auto& lightComp = GetOwnerEntity()->GetComponent<LightComponent>();
		switch (lightComp.Type)
		{
			case LightComponent::LightType::DirectionalLight:	lightComp.LightSource = CreateRef<DirectionalLight>(*std::dynamic_pointer_cast<DirectionalLight>(otherLightComp->LightSource)); break;
			case LightComponent::LightType::PointLight:			lightComp.LightSource = CreateRef<PointLight>(*std::dynamic_pointer_cast<PointLight>(otherLightComp->LightSource)); break;
			case LightComponent::LightType::SpotLight:			lightComp.LightSource = CreateRef<SpotLight>(*std::dynamic_pointer_cast<SpotLight>(otherLightComp->LightSource)); break;
			default: break;
		}
	}

	void LightComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue)
	{
		if (dataId == ZDATA_ID(Type))
		{
			InitLight();
		}
	}

	void LightComponentHelper::InitLight()
	{
		auto& lightComp = GetOwnerEntity()->GetComponent<LightComponent>();
		auto& billboardComp = GetOwnerEntity()->GetComponent<BillboardComponent>();
		switch (lightComp.Type)
		{
			case LightComponent::LightType::DirectionalLight:
				lightComp.LightSource = CreateRef<DirectionalLight>();
				billboardComp.Texture = Texture2DAssetLibrary::Get().LoadAsset("resources/textures/icons/DirectionalLight.png.zasset");
				break;
			case LightComponent::LightType::PointLight:
				lightComp.LightSource = CreateRef<PointLight>();
				billboardComp.Texture = Texture2DAssetLibrary::Get().LoadAsset("resources/textures/icons/PointLight.png.zasset");
				break;
			case LightComponent::LightType::SpotLight:
				lightComp.LightSource = CreateRef<SpotLight>();
				billboardComp.Texture = Texture2DAssetLibrary::Get().LoadAsset("resources/textures/icons/SpotLight.png.zasset");
				break;
			default:
				break;
		}
	}

}
