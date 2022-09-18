#include "ZEpch.h"
#include "Engine/GameFramework/ComponentHelpers.h"

#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scripting/ScriptEngine.h"

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

#pragma region TransformComponentHelper
	void TransformComponentHelper::OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		GetOwnerEntity()->UpdateBounds();
	}

	void TransformComponentHelper::PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		GetOwnerEntity()->UpdateBounds();
	}
#pragma endregion

#pragma region CameraComponentHelper
	void CameraComponentHelper::OnComponentAdded(bool bIsDeserialize)
	{
		auto& billboardComp = GetOwnerEntity()->AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/Camera.png.zasset");
	}

	void CameraComponentHelper::OnComponentDestroy()
	{
		GetOwnerEntity()->RemoveComponentIfExist<BillboardComponent>();
	}
#pragma endregion

#pragma region ScriptComponentHelper
	void ScriptComponentHelper::OnComponentDestroy()
	{
		ScriptEngine::OnDestroyEntity(*GetOwnerEntity());
	}

	void ScriptComponentHelper::PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		if (dataId == GetDataIdByName<ScriptComponent>("ClassName"))
		{
			ScriptEngine::InitScriptEntity(*GetOwnerEntity());
		}
	}

	void ScriptComponentHelper::PostDataDeserialize(U32 dataId)
	{
		if (dataId == GetDataIdByName<ScriptComponent>("ClassName"))
		{
			ScriptEngine::InitScriptEntity(*GetOwnerEntity());
		}
	}
#pragma endregion

#pragma region ParticleSystemComponentHelper
	void ParticleSystemComponentHelper::OnComponentCopied(IComponent* otherComp)
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		ParticleSystemInstance::Create(particleComp);
	}

	void ParticleSystemComponentHelper::OnComponentDestroy()
	{
		const auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
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
#pragma endregion

#pragma region ParticleSystemPreviewComponentHelper
	void ParticleSystemPreviewComponentHelper::OnComponentDestroy()
	{
		const auto& particlePreviewComp = GetOwnerEntity()->GetComponent<ParticleSystemPreviewComponent>();
		if (particlePreviewComp.ParticleTemplateAsset)
		{
			particlePreviewComp.ParticleTemplateAsset->RemoveParticleSystemInstance(particlePreviewComp.Instance);
		}
	}

	void ParticleSystemPreviewComponentHelper::OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		const auto& particlePreviewComp = GetOwnerEntity()->GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.ParticleTemplateAsset->ResimulateAllParticleSystemInstances();
	}
	 
	void ParticleSystemPreviewComponentHelper::PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		const auto& particlePreviewComp = GetOwnerEntity()->GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.ParticleTemplateAsset->ResimulateAllParticleSystemInstances();
	}
#pragma endregion

#pragma region MeshRendererComponentHelper
	void MeshRendererComponentHelper::OnComponentAdded(bool bIsDeserialize)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		if (meshComp.MeshAsset)
		{
			meshComp.Instance = meshComp.MeshAsset->CreateInstance(GetOwnerEntity()->GetScene(), bIsDeserialize);
		}
	}

	void MeshRendererComponentHelper::OnComponentCopied(IComponent* otherComp)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		MeshInstance::Copy(meshComp, static_cast<MeshRendererComponent*>(otherComp)->Instance);
	}

	void MeshRendererComponentHelper::PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		if (dataId == GetDataIdByName<MeshRendererComponent>("MeshAsset"))
		{
			GetOwnerEntity()->UpdateBounds();
			meshComp.Instance = meshComp.MeshAsset ? meshComp.MeshAsset->CreateInstance(GetOwnerEntity()->GetScene()) : nullptr;
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
			// Create mesh instance when mesh asset is loaded so that material data can be deserialized into mesh instance properly
			if (meshComp.MeshAsset)
			{
				meshComp.Instance = meshComp.MeshAsset->CreateInstance(GetOwnerEntity()->GetScene());
			}
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
#pragma endregion

#pragma region MeshPreviewComponentHelper
	void MeshPreviewComponentHelper::PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshPreviewComponent>();
		if (dataId == GetDataIdByName<MeshPreviewComponent>("MaterialSlots"))
		{
			meshComp.Instance->SetMaterial(elementIndex, meshComp.GetMaterials()[elementIndex]);
			auto oldMaterial = (*oldValue._Cast<Ref<Material>>());
			meshComp.Instance->OnMaterialChanged(elementIndex, oldMaterial);
		}
	}

	BoxSphereBounds MeshPreviewComponentHelper::GetBounds()
	{
		auto& transformComp = GetOwnerEntity()->GetComponent<TransformComponent>();
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshPreviewComponent>();
		return meshComp.MeshAsset ? meshComp.MeshAsset->GetBounds().TransformBy(transformComp.GetTransform()) : BoxSphereBounds{};
	}

	std::string MeshPreviewComponentHelper::GetCustomSequenceContainerElementName(U32 index) const
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshPreviewComponent>();
		return meshComp.MeshAsset->GetMaterialNames()[index];
	}
#pragma endregion

#pragma region LightComponentHelper
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
			case LightComponent::LightType::DirectionalLight:	lightComp.LightSource = CreateRef<DirectionalLight>(*std::static_pointer_cast<DirectionalLight>(otherLightComp->LightSource)); break;
			case LightComponent::LightType::PointLight:			lightComp.LightSource = CreateRef<PointLight>(*std::static_pointer_cast<PointLight>(otherLightComp->LightSource)); break;
			case LightComponent::LightType::SpotLight:			lightComp.LightSource = CreateRef<SpotLight>(*std::static_pointer_cast<SpotLight>(otherLightComp->LightSource)); break;
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
		const float range = lightComp.Type == LightComponent::LightType::DirectionalLight ? 0.0f : lightComp.LightSource->GetRange();
		const Sphere sphere{ transformComp.Translation, range * (lightComp.Type == LightComponent::LightType::SpotLight ? 0.5f : 1.0f) };
		return sphere;
	}

	void LightComponentHelper::InitLight() const
	{
		auto& lightComp = GetOwnerEntity()->GetComponent<LightComponent>();
		auto& billboardComp = GetOwnerEntity()->GetComponent<BillboardComponent>();
		switch (lightComp.Type)
		{
			case LightComponent::LightType::DirectionalLight:
				lightComp.LightSource = CreateRef<DirectionalLight>(GetOwnerEntity()->GetScene());
				billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/DirectionalLight.png.zasset");
				break;
			case LightComponent::LightType::PointLight:
				lightComp.LightSource = CreateRef<PointLight>(GetOwnerEntity()->GetScene());
				billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/PointLight.png.zasset");
				break;
			case LightComponent::LightType::SpotLight:
				lightComp.LightSource = CreateRef<SpotLight>(GetOwnerEntity()->GetScene());
				billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/SpotLight.png.zasset");
				break;
			default:
				break;
		}
	}
#pragma endregion

}
