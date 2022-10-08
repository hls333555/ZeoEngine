#include "ZEpch.h"
#include "Engine/GameFramework/ComponentHelpers.h"

#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scripting/ScriptEngine.h"

namespace ZeoEngine {

	/** Map from component ID to its helper */
	static std::unordered_map<U32, Scope<IComponentHelper>> s_ComponentHelpers;

	std::vector<AssetHandle> g_AssetVectorPlaceholder;

	void ComponentHelperRegistry::AddComponentHelper(U32 compID, Scope<IComponentHelper> helper)
	{
		s_ComponentHelpers[compID] = std::move(helper);
	}

	IComponentHelper* ComponentHelperRegistry::GetComponentHelper(U32 compID)
	{
		return s_ComponentHelpers.find(compID) != s_ComponentHelpers.end() ? s_ComponentHelpers[compID].get() : nullptr;
	}

#pragma region TransformComponentHelper
	void TransformComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		comp->OwnerEntity.UpdateBounds();
	}
#pragma endregion

#pragma region CameraComponentHelper
	void CameraComponentHelper::OnComponentAdded(IComponent* comp, bool bIsDeserialize)
	{
		auto& billboardComp = comp->OwnerEntity.AddComponent<BillboardComponent>();
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/Camera.png.zasset");
	}

	void CameraComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		auto* cameraComp = static_cast<CameraComponent*>(comp);
		if (fieldID == GetFieldIDByName<CameraComponent>("ProjectionType") ||
			fieldID == GetFieldIDByName<CameraComponent>("VerticalFOV") ||
			fieldID == GetFieldIDByName<CameraComponent>("PerspectiveNear") ||
			fieldID == GetFieldIDByName<CameraComponent>("PerspectiveFar") ||
			fieldID == GetFieldIDByName<CameraComponent>("Size") ||
			fieldID == GetFieldIDByName<CameraComponent>("OrthographicNear") ||
			fieldID == GetFieldIDByName<CameraComponent>("OrthographicFar"))
		{
			cameraComp->Camera.RecalculateProjection();
		}
	}

	void CameraComponentHelper::PostFieldDeserialize(IComponent* comp, U32 fieldID)
	{
		auto* cameraComp = static_cast<CameraComponent*>(comp);
		if (fieldID == GetFieldIDByName<CameraComponent>("ProjectionType") ||
			fieldID == GetFieldIDByName<CameraComponent>("VerticalFOV") ||
			fieldID == GetFieldIDByName<CameraComponent>("PerspectiveNear") ||
			fieldID == GetFieldIDByName<CameraComponent>("PerspectiveFar") ||
			fieldID == GetFieldIDByName<CameraComponent>("Size") ||
			fieldID == GetFieldIDByName<CameraComponent>("OrthographicNear") ||
			fieldID == GetFieldIDByName<CameraComponent>("OrthographicFar"))
		{
			cameraComp->Camera.RecalculateProjection();
		}
	}

	void CameraComponentHelper::OnComponentDestroy(IComponent* comp)
	{
		comp->OwnerEntity.RemoveComponentIfExist<BillboardComponent>();
	}
#pragma endregion

#pragma region ScriptComponentHelper
	void ScriptComponentHelper::OnComponentDestroy(IComponent* comp)
	{
		ScriptEngine::OnDestroyEntity(comp->OwnerEntity);
	}

	void ScriptComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		if (fieldID == GetFieldIDByName<ScriptComponent>("ClassName"))
		{
			ScriptEngine::InitScriptEntity(comp->OwnerEntity);
		}
	}

	void ScriptComponentHelper::PostFieldDeserialize(IComponent* comp, U32 fieldID)
	{
		if (fieldID == GetFieldIDByName<ScriptComponent>("ClassName"))
		{
			ScriptEngine::InitScriptEntity(comp->OwnerEntity);
		}
	}
#pragma endregion

#pragma region ParticleSystemComponentHelper
	void ParticleSystemComponentHelper::OnComponentCopied(IComponent* comp, IComponent* otherComp)
	{
		auto* particleComp = static_cast<ParticleSystemComponent*>(comp);
		ParticleSystemInstance::Create(*particleComp);
	}

	void ParticleSystemComponentHelper::OnComponentDestroy(IComponent* comp)
	{
		const auto* particleComp = static_cast<ParticleSystemComponent*>(comp);
		if (particleComp->ParticleTemplateAsset)
		{
			particleComp->ParticleTemplateAsset->RemoveParticleSystemInstance(particleComp->Instance);
		}
	}

	void ParticleSystemComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		auto* particleComp = static_cast<ParticleSystemComponent*>(comp);
		if (fieldID == GetFieldIDByName<ParticleSystemComponent>("ParticleTemplateAsset"))
		{
			Ref<ParticleTemplate> oldTemplate = *static_cast<const Ref<ParticleTemplate>*>(oldValue);
			if (oldTemplate)
			{
				oldTemplate->RemoveParticleSystemInstance(particleComp->Instance);
			}
		}
		// Manually clear particle template selection
		if (!particleComp->ParticleTemplateAsset)
		{
			particleComp->Instance.reset();
		}
		else
		{
			ParticleSystemInstance::Create(*particleComp);
		}
	}
#pragma endregion

#pragma region ParticleSystemPreviewComponentHelper
	void ParticleSystemPreviewComponentHelper::OnComponentDestroy(IComponent* comp)
	{
		const auto* particlePreviewComp = static_cast<ParticleSystemPreviewComponent*>(comp);
		if (particlePreviewComp->ParticleTemplateAsset)
		{
			particlePreviewComp->ParticleTemplateAsset->RemoveParticleSystemInstance(particlePreviewComp->Instance);
		}
	}
	 
	void ParticleSystemPreviewComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		const auto* particlePreviewComp = static_cast<ParticleSystemPreviewComponent*>(comp);
		particlePreviewComp->ParticleTemplateAsset->ResimulateAllParticleSystemInstances();
	}
#pragma endregion

#pragma region MeshRendererComponentHelper
	void MeshRendererComponentHelper::OnComponentAdded(IComponent* comp, bool bIsDeserialize)
	{
		auto* meshComp = static_cast<MeshRendererComponent*>(comp);
		const auto mesh = meshComp->GetMesh();
		meshComp->Instance = mesh ? mesh->CreateInstance(comp->OwnerEntity.GetScene(), bIsDeserialize) : nullptr;
	}

	void MeshRendererComponentHelper::OnComponentCopied(IComponent* comp, IComponent* otherComp)
	{
		auto* meshComp = static_cast<MeshRendererComponent*>(comp);
		MeshInstance::Copy(*meshComp, static_cast<MeshRendererComponent*>(otherComp)->Instance);
	}

	void MeshRendererComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		auto* meshComp = static_cast<MeshRendererComponent*>(comp);
		Entity entity = comp->OwnerEntity;
		if (fieldID == GetFieldIDByName<MeshRendererComponent>("MeshAsset"))
		{
			entity.UpdateBounds();
			const auto mesh = meshComp->GetMesh();
			meshComp->Instance = mesh ? mesh->CreateInstance(entity.GetScene()) : nullptr;
		}
		else if (fieldID == GetFieldIDByName<MeshRendererComponent>("MaterialSlots"))
		{
			const auto oldMaterial = *static_cast<const AssetHandle*>(oldValue);
			meshComp->Instance->OnMaterialChanged(elementIndex, oldMaterial);
		}
	}

	void MeshRendererComponentHelper::PostFieldDeserialize(IComponent* comp, U32 fieldID)
	{
		auto* meshComp = static_cast<MeshRendererComponent*>(comp);
		if (fieldID == GetFieldIDByName<MeshRendererComponent>("MeshAsset"))
		{
			// Create mesh instance when mesh asset is loaded so that material data can be deserialized into mesh instance properly
			const auto mesh = meshComp->GetMesh();
			meshComp->Instance = mesh ? mesh->CreateInstance(comp->OwnerEntity.GetScene()) : nullptr;
		}
		else if (fieldID == GetFieldIDByName<MeshRendererComponent>("MaterialSlots"))
		{
			meshComp->Instance->SubmitAllTechniques();
		}
	}

	BoxSphereBounds MeshRendererComponentHelper::GetBounds(IComponent* comp)
	{
		const auto& transformComp = comp->OwnerEntity.GetComponent<TransformComponent>();
		const auto* meshComp = static_cast<MeshRendererComponent*>(comp);
		const auto mesh = meshComp->GetMesh();
		return mesh ? mesh->GetBounds().TransformBy(transformComp.GetTransform()) : BoxSphereBounds{};
	}

	std::string MeshRendererComponentHelper::GetCustomSequenceContainerElementName(IComponent* comp, U32 index) const
	{
		const auto* meshComp = static_cast<MeshRendererComponent*>(comp);
		return meshComp->GetMesh()->GetMaterialNames()[index];
	}
#pragma endregion

#pragma region MeshPreviewComponentHelper
	void MeshPreviewComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		const auto* meshComp = static_cast<MeshPreviewComponent*>(comp);
		if (fieldID == GetFieldIDByName<MeshPreviewComponent>("MaterialSlots"))
		{
			meshComp->Instance->SetMaterial(elementIndex, meshComp->GetMaterialAssets()[elementIndex]);
			const auto oldMaterial = *static_cast<const AssetHandle*>(oldValue);
			meshComp->Instance->OnMaterialChanged(elementIndex, oldMaterial);
		}
	}

	BoxSphereBounds MeshPreviewComponentHelper::GetBounds(IComponent* comp)
	{
		const auto& transformComp = comp->OwnerEntity.GetComponent<TransformComponent>();
		const auto* meshComp = static_cast<MeshPreviewComponent*>(comp);
		const auto& mesh = meshComp->LoadedMesh;
		return mesh ? mesh->GetBounds().TransformBy(transformComp.GetTransform()) : BoxSphereBounds{};
	}

	std::string MeshPreviewComponentHelper::GetCustomSequenceContainerElementName(IComponent* comp, U32 index) const
	{
		const auto* meshComp = static_cast<MeshPreviewComponent*>(comp);
		return meshComp->LoadedMesh->GetMaterialNames()[index];
	}
#pragma endregion

#pragma region MaterialPreviewComponentHelper
	void MaterialPreviewComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		const auto* materialComp = static_cast<MaterialPreviewComponent*>(comp);
		if (fieldID == GetFieldIDByName<MaterialPreviewComponent>("ShaderAsset"))
		{
			const auto oldShader = *static_cast<const AssetHandle*>(oldValue);
			materialComp->LoadedMaterial->OnShaderChanged(materialComp->GetShaderAsset(), oldShader);
		}
	}

	void MaterialPreviewComponentHelper::PostFieldDeserialize(IComponent* comp, U32 fieldID)
	{
		const auto* materialComp = static_cast<MaterialPreviewComponent*>(comp);
		if (fieldID == GetFieldIDByName<MaterialPreviewComponent>("ShaderAsset"))
		{
			materialComp->LoadedMaterial->OnShaderChanged(materialComp->GetShaderAsset(), 0);
		}
	}
#pragma endregion

#pragma region TexturePreviewComponentHelper
	void TexturePreviewComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		const auto* textureComp = static_cast<TexturePreviewComponent*>(comp);
		if (fieldID == GetFieldIDByName<TexturePreviewComponent>("SRGB") ||
			fieldID == GetFieldIDByName<TexturePreviewComponent>("GenerateMipmaps"))
		{
			textureComp->LoadedTexture->Invalidate();
		}
		else if (fieldID == GetFieldIDByName<TexturePreviewComponent>("SamplerType"))
		{
			textureComp->LoadedTexture->ChangeSampler(textureComp->SamplerType);
		}
	}
#pragma endregion

#pragma region LightComponentHelper
	void LightComponentHelper::OnComponentAdded(IComponent* comp, bool bIsDeserialize)
	{
		Entity entity = comp->OwnerEntity;
		entity.AddComponent<BillboardComponent>();
		InitLight(&entity);
	}

	void LightComponentHelper::OnComponentCopied(IComponent* comp, IComponent* otherComp)
	{
		// Perform the deep copy of smart pointer
		const auto* otherLightComp = static_cast<LightComponent*>(otherComp);
		auto* lightComp = static_cast<LightComponent*>(comp);
		switch (lightComp->Type)
		{
			case LightComponent::LightType::DirectionalLight:	lightComp->LightSource = CreateRef<DirectionalLight>(*std::static_pointer_cast<DirectionalLight>(otherLightComp->LightSource)); break;
			case LightComponent::LightType::PointLight:			lightComp->LightSource = CreateRef<PointLight>(*std::static_pointer_cast<PointLight>(otherLightComp->LightSource)); break;
			case LightComponent::LightType::SpotLight:			lightComp->LightSource = CreateRef<SpotLight>(*std::static_pointer_cast<SpotLight>(otherLightComp->LightSource)); break;
		}
	}

	void LightComponentHelper::OnComponentDestroy(IComponent* comp)
	{
		comp->OwnerEntity.RemoveComponentIfExist<BillboardComponent>();
	}

	void LightComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		if (fieldID == GetFieldIDByName<LightComponent>("Type"))
		{
			InitLight(&comp->OwnerEntity);
		}
		else if (fieldID == GetFieldIDByName<LightComponent>("Range"))
		{
			comp->OwnerEntity.UpdateBounds();
		}
		else if (fieldID == GetFieldIDByName<LightComponent>("ShadowType"))
		{
			const auto* lightComp = static_cast<LightComponent*>(comp);
			lightComp->LightSource->OnShadowTypeChanged();
		}
	}

	void LightComponentHelper::PostFieldDeserialize(IComponent* comp, U32 fieldID)
	{
		// Create light instance when light type is loaded so that light specific data can be deserialized properly
		if (fieldID == GetFieldIDByName<LightComponent>("Type"))
		{
			InitLight(&comp->OwnerEntity);
		}
		else if (fieldID == GetFieldIDByName<LightComponent>("ShadowType"))
		{
			const auto* lightComp = static_cast<LightComponent*>(comp);
			lightComp->LightSource->OnShadowTypeChanged();
		}
	}

	BoxSphereBounds LightComponentHelper::GetBounds(IComponent* comp)
	{
		const auto* lightComp = static_cast<LightComponent*>(comp);
		const auto& transformComp = comp->OwnerEntity.GetComponent<TransformComponent>();
		const float range = lightComp->Type == LightComponent::LightType::DirectionalLight ? 0.0f : lightComp->LightSource->GetRange();
		const Sphere sphere{ transformComp.Translation, range * (lightComp->Type == LightComponent::LightType::SpotLight ? 0.5f : 1.0f) };
		return sphere;
	}

	void LightComponentHelper::InitLight(Entity* entity) const
	{
		auto& lightComp = entity->GetComponent<LightComponent>();
		auto& billboardComp = entity->GetComponent<BillboardComponent>();
		switch (lightComp.Type)
		{
			case LightComponent::LightType::DirectionalLight:
				lightComp.LightSource = CreateRef<DirectionalLight>(entity->GetScene());
				billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/DirectionalLight.png.zasset");
				break;
			case LightComponent::LightType::PointLight:
				lightComp.LightSource = CreateRef<PointLight>(entity->GetScene());
				billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/PointLight.png.zasset");
				break;
			case LightComponent::LightType::SpotLight:
				lightComp.LightSource = CreateRef<SpotLight>(entity->GetScene());
				billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/SpotLight.png.zasset");
				break;
		}
	}
#pragma endregion

}
