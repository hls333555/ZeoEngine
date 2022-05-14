#include "ZEpch.h"
#include "Engine/GameFramework/ComponentHelpers.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/ReflectionCore.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Utils/EngineUtils.h"

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

	void TransformComponentHelper::OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex)
	{
		GetOwnerEntity()->UpdateBounds();
	}

	void TransformComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex)
	{
		GetOwnerEntity()->UpdateBounds();
	}

	void CameraComponentHelper::OnComponentAdded(bool bIsDeserialize)
	{
		auto& billboardComp = GetOwnerEntity()->AddComponent<BillboardComponent>();
		billboardComp.Texture = Texture2DLibrary::Get().LoadAsset("assets/editor/textures/icons/Camera.png.zasset");
	}

	void CameraComponentHelper::OnComponentDestroy()
	{
		GetOwnerEntity()->RemoveComponent<BillboardComponent>();
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

	void ParticleSystemComponentHelper::OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex)
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		ParticleSystemInstance::Create(particleComp);
	}

	void ParticleSystemComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex)
	{
		auto& particleComp = GetOwnerEntity()->GetComponent<ParticleSystemComponent>();
		if (dataId == ZDATA_ID(Template))
		{
			AssetHandle<ParticleTemplate> oldTemplate = (*oldValue._Cast<AssetHandle<ParticleTemplate>>());
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

	void ParticleSystemPreviewComponentHelper::OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex)
	{
		auto& particlePreviewComp = GetOwnerEntity()->GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.Template->ResimulateAllParticleSystemInstances();
	}
	 
	void ParticleSystemPreviewComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex)
	{
		auto& particlePreviewComp = GetOwnerEntity()->GetComponent<ParticleSystemPreviewComponent>();
		particlePreviewComp.Template->ResimulateAllParticleSystemInstances();
	}

	const RenderGraph& MeshRendererComponentHelper::GetRenderGraph(Entity* entityContext) const
	{
		return EngineUtils::GetSceneRendererFromContext(entityContext)->GetRenderGraph();
	}

	void MeshRendererComponentHelper::OnComponentAdded(bool bIsDeserialize)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		MeshInstance::Create(meshComp, GetRenderGraph(GetOwnerEntity()), nullptr, bIsDeserialize);
	}

	void MeshRendererComponentHelper::OnComponentCopied(IComponent* otherComp)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		// When component is copied during scene copy, the active scene is not updated yet in EditorBase,
		// therefore the render graph retrieved from the copied entity will be nullptr.
		// So we use the original entity to retrieve that,
		// which works because we assume the scene renderer is shared with the copied scene
		MeshInstance::Create(meshComp, GetRenderGraph(otherComp->ComponentHelper->GetOwnerEntity()), static_cast<MeshRendererComponent*>(otherComp)->Instance);
	}

	void MeshRendererComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		if (dataId == ZDATA_ID(Mesh))
		{
			GetOwnerEntity()->UpdateBounds();
			if (meshComp.Mesh)
			{
				MeshInstance::Create(meshComp, GetRenderGraph(GetOwnerEntity()));
			}
			else
			{
				meshComp.Instance = nullptr;
			}
		}
		else if (dataId == ZDATA_ID(MaterialSlots))
		{
			auto oldMaterial = (*oldValue._Cast<AssetHandle<Material>>());
			meshComp.Instance->OnMaterialChanged(elementIndex, oldMaterial);
		}
	}

	void MeshRendererComponentHelper::PostDataDeserialize(uint32_t dataId)
	{
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		if (dataId == ZDATA_ID(Mesh))
		{
			// Create mesh instance when mesh asset is loaded so that material data can be deserizlized properly
			MeshInstance::Create(meshComp, GetRenderGraph(GetOwnerEntity()));
		}
		else if (dataId == ZDATA_ID(MaterialSlots))
		{
			meshComp.Instance->SubmitAllTechniques();
		}
	}

	BoxSphereBounds MeshRendererComponentHelper::GetBounds()
	{
		auto& transformComp = GetOwnerEntity()->GetComponent<TransformComponent>();
		auto& meshComp = GetOwnerEntity()->GetComponent<MeshRendererComponent>();
		return meshComp.Mesh ? meshComp.Mesh->GetMesh()->GetBounds().TransformBy(transformComp.GetTransform()) : BoxSphereBounds{};
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
		GetOwnerEntity()->RemoveComponent<BillboardComponent>();
	}

	void LightComponentHelper::OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex)
	{
		if (dataId == ZDATA_ID(Range))
		{
			GetOwnerEntity()->UpdateBounds();
		}
	}

	void LightComponentHelper::PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex)
	{
		if (dataId == ZDATA_ID(Type))
		{
			InitLight();
		}
		else if (dataId == ZDATA_ID(Range))
		{
			GetOwnerEntity()->UpdateBounds();
		}
	}

	void LightComponentHelper::PostDataDeserialize(uint32_t dataId)
	{
		// Create light instance when light type is loaded so that light specific data can be deserizlized properly
		auto& lightComp = GetOwnerEntity()->GetComponent<LightComponent>();
		if (dataId == ZDATA_ID(Type))
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
				billboardComp.Texture = Texture2DLibrary::Get().LoadAsset("assets/editor/textures/icons/DirectionalLight.png.zasset");
				break;
			case LightComponent::LightType::PointLight:
				lightComp.LightSource = CreateRef<PointLight>();
				billboardComp.Texture = Texture2DLibrary::Get().LoadAsset("assets/editor/textures/icons/PointLight.png.zasset");
				break;
			case LightComponent::LightType::SpotLight:
				lightComp.LightSource = CreateRef<SpotLight>();
				billboardComp.Texture = Texture2DLibrary::Get().LoadAsset("assets/editor/textures/icons/SpotLight.png.zasset");
				break;
			default:
				break;
		}
	}

}
