#include "ZEpch.h"
#include "Engine/GameFramework/ComponentHelpers.h"

#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/SceneRenderer.h"

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

	void TransformComponentHelper::OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		GetOwnerEntity()->UpdateBounds();
	}

	void TransformComponentHelper::PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		GetOwnerEntity()->UpdateBounds();
	}

	void CameraComponentHelper::OnComponentAdded(bool bIsDeserialize)
	{
		auto& billboardComp = GetOwnerEntity()->AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/editor/textures/icons/Camera.png.zasset");
	}

	void CameraComponentHelper::OnComponentDestroy()
	{
		GetOwnerEntity()->RemoveComponentIfExist<BillboardComponent>();
	}

	void ParticleSystemComponentHelper::OnComponentCopied(IComponent* otherComp)
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		ParticleSystemInstance::Create(particleComp);
	}

	void ParticleSystemComponentHelper::OnComponentDestroy()
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		if (particleComp.ParticleTemplateAsset)
		{
			particleComp.ParticleTemplateAsset->RemoveParticleSystemInstance(particleComp.Instance);
		}
	}

	void ParticleSystemComponentHelper::OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		ParticleSystemInstance::Create(particleComp);
	}

	void ParticleSystemComponentHelper::PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		if (dataId == GetDataIdByName<ParticleSystemComponent>("ParticleTemplateAsset"))
		{
			Ref<ParticleTemplate> oldTemplate = (*oldValue._Cast<Ref<ParticleTemplate>>());
			if (oldTemplate)
			{
				oldTemplate->RemoveParticleSystemInstance(particleComp.Instance);
			}
		}
		// Manually clear particle template selection
		if (!particleComp.ParticleTemplateAsset)
		{
			particleComp.Instance.reset();
		}
		else
		{
			ParticleSystemInstance::Create(particleComp);
		}
	}

	void ParticleSystemPreviewComponentHelper::OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		auto& particlePreviewComp = GetOwnerEntity()->GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.ParticleTemplateAsset->ResimulateAllParticleSystemInstances();
	}
	 
	void ParticleSystemPreviewComponentHelper::PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		auto& particlePreviewComp = GetOwnerEntity()->GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.ParticleTemplateAsset->ResimulateAllParticleSystemInstances();
	}

	void MeshRendererComponentHelper::OnComponentAdded(bool bIsDeserialize)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		MeshInstance::Create(GetOwnerEntity()->GetScene(), meshComp, bIsDeserialize);
	}

	void MeshRendererComponentHelper::OnComponentCopied(IComponent* otherComp)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		MeshInstance::Copy(meshComp, dynamic_cast<MeshRendererComponent*>(otherComp)->Instance);
	}

	void MeshRendererComponentHelper::PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		if (dataId == GetDataIdByName<MeshRendererComponent>("MeshAsset"))
		{
			GetOwnerEntity()->UpdateBounds();
			if (meshComp.MeshAsset)
			{
				MeshInstance::Create(GetOwnerEntity()->GetScene(), meshComp);
			}
			else
			{
				meshComp.Instance = nullptr;
			}
		}
		else if (dataId == GetDataIdByName<MeshRendererComponent>("MaterialSlots"))
		{
			auto oldMaterial = (*oldValue._Cast<Ref<Material>>());
			meshComp.Instance->OnMaterialChanged(elementIndex, oldMaterial);
		}
	}

	void MeshRendererComponentHelper::PostDataDeserialize(U32 dataId)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		if (dataId == GetDataIdByName<MeshRendererComponent>("MeshAsset"))
		{
			// Create mesh instance when mesh asset is loaded so that material data can be deserizlized properly
			MeshInstance::Create(GetOwnerEntity()->GetScene(), meshComp);
		}
		else if (dataId == GetDataIdByName<MeshRendererComponent>("MaterialSlots"))
		{
			meshComp.Instance->SubmitAllTechniques();
		}
	}

	BoxSphereBounds MeshRendererComponentHelper::GetBounds()
	{
		auto& transformComp = GetOwnerEntity()->GetComponent<TransformComponent>();
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		return meshComp.MeshAsset ? meshComp.MeshAsset->GetBounds().TransformBy(transformComp.GetTransform()) : BoxSphereBounds{};
	}

	std::string MeshRendererComponentHelper::GetCustomSequenceContainerElementName(U32 index) const
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		return meshComp.MeshAsset->GetMaterialNames()[index];
	}

	void LightComponentHelper::OnComponentAdded(bool bIsDeserialize)
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

	void LightComponentHelper::OnComponentDestroy()
	{
		GetOwnerEntity()->RemoveComponentIfExist<BillboardComponent>();
	}

	void LightComponentHelper::OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		if (dataId == GetDataIdByName<LightComponent>("Range"))
		{
			GetOwnerEntity()->UpdateBounds();
		}
	}

	void LightComponentHelper::PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		if (dataId == GetDataIdByName<LightComponent>("Type"))
		{
			InitLight();
		}
		else if (dataId == GetDataIdByName<LightComponent>("Range"))
		{
			GetOwnerEntity()->UpdateBounds();
		}
	}

	void LightComponentHelper::PostDataDeserialize(U32 dataId)
	{
		// Create light instance when light type is loaded so that light specific data can be deserizlized properly
		auto& lightComp = GetOwnerEntity()->GetComponent<LightComponent>();
		if (dataId == GetDataIdByName<LightComponent>("Type"))
		{
			lightComp.GetHelper<LightComponentHelper>()->InitLight();
		}
	}

	BoxSphereBounds LightComponentHelper::GetBounds()
	{
		auto& lightComp = GetOwnerEntity()->GetComponent<LightComponent>();
		auto& transformComp = GetOwnerEntity()->GetComponent<TransformComponent>();
		Sphere sphere{ transformComp.Translation, lightComp.LightSource->GetRange() * (lightComp.Type == LightComponent::LightType::SpotLight ? 0.5f : 1.0f) };
		return sphere;
	}

	void LightComponentHelper::InitLight()
	{
		auto& lightComp = GetOwnerEntity()->GetComponent<LightComponent>();
		auto& billboardComp = GetOwnerEntity()->GetComponent<BillboardComponent>();
		switch (lightComp.Type)
		{
			case LightComponent::LightType::DirectionalLight:
				lightComp.LightSource = CreateRef<DirectionalLight>();
				billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/editor/textures/icons/DirectionalLight.png.zasset");
				break;
			case LightComponent::LightType::PointLight:
				lightComp.LightSource = CreateRef<PointLight>();
				billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/editor/textures/icons/PointLight.png.zasset");
				break;
			case LightComponent::LightType::SpotLight:
				lightComp.LightSource = CreateRef<SpotLight>();
				billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/editor/textures/icons/SpotLight.png.zasset");
				break;
			default:
				break;
		}
	}

}
