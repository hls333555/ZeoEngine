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
			ScriptEngine::OnScriptClassChanged(comp->OwnerEntity);
		}
	}

	void ScriptComponentHelper::PostFieldDeserialize(IComponent* comp, U32 fieldID)
	{
		if (fieldID == GetFieldIDByName<ScriptComponent>("ClassName"))
		{
			ScriptEngine::OnScriptClassChanged(comp->OwnerEntity);
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

#pragma region ParticleSystemDetailComponentHelper
	void ParticleSystemDetailComponentHelper::OnComponentDestroy(IComponent* comp)
	{
		const auto* particlePreviewComp = static_cast<ParticleSystemDetailComponent*>(comp);
		if (particlePreviewComp->ParticleTemplateAsset)
		{
			particlePreviewComp->ParticleTemplateAsset->RemoveParticleSystemInstance(particlePreviewComp->Instance);
		}
	}
	 
	void ParticleSystemDetailComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		const auto* particlePreviewComp = static_cast<ParticleSystemDetailComponent*>(comp);
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

#pragma region MeshDetailComponentHelper
	void MeshDetailComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		const auto* meshComp = static_cast<MeshDetailComponent*>(comp);
		if (fieldID == GetFieldIDByName<MeshDetailComponent>("MaterialSlots"))
		{
			meshComp->Instance->SetMaterial(elementIndex, meshComp->GetMaterialAssets()[elementIndex]);
			const auto oldMaterial = *static_cast<const AssetHandle*>(oldValue);
			meshComp->Instance->OnMaterialChanged(elementIndex, oldMaterial);
		}
	}

	BoxSphereBounds MeshDetailComponentHelper::GetBounds(IComponent* comp)
	{
		const auto& transformComp = comp->OwnerEntity.GetComponent<TransformComponent>();
		const auto* meshComp = static_cast<MeshDetailComponent*>(comp);
		const auto& mesh = meshComp->LoadedMesh;
		return mesh ? mesh->GetBounds().TransformBy(transformComp.GetTransform()) : BoxSphereBounds{};
	}

	std::string MeshDetailComponentHelper::GetCustomSequenceContainerElementName(IComponent* comp, U32 index) const
	{
		const auto* meshComp = static_cast<MeshDetailComponent*>(comp);
		return meshComp->LoadedMesh->GetMaterialNames()[index];
	}
#pragma endregion

#pragma region MaterialDetailComponentHelper
	void MaterialDetailComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		const auto* materialComp = static_cast<MaterialDetailComponent*>(comp);
		if (fieldID == GetFieldIDByName<MaterialDetailComponent>("ShaderAsset"))
		{
			const auto oldShader = *static_cast<const AssetHandle*>(oldValue);
			materialComp->LoadedMaterial->OnShaderChanged(materialComp->GetShaderAsset(), oldShader);
		}
	}

	void MaterialDetailComponentHelper::PostFieldDeserialize(IComponent* comp, U32 fieldID)
	{
		const auto* materialComp = static_cast<MaterialDetailComponent*>(comp);
		if (fieldID == GetFieldIDByName<MaterialDetailComponent>("ShaderAsset"))
		{
			materialComp->LoadedMaterial->OnShaderChanged(materialComp->GetShaderAsset(), 0);
		}
	}
#pragma endregion

#pragma region TextureDetailComponentHelper
	void TextureDetailComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		const auto* textureComp = static_cast<TextureDetailComponent*>(comp);
		if (fieldID == GetFieldIDByName<TextureDetailComponent>("SRGB") ||
			fieldID == GetFieldIDByName<TextureDetailComponent>("GenerateMipmaps"))
		{
			textureComp->LoadedTexture->Invalidate();
		}
		else if (fieldID == GetFieldIDByName<TextureDetailComponent>("SamplerType"))
		{
			textureComp->LoadedTexture->ChangeSampler(textureComp->SamplerType);
		}
	}
#pragma endregion

#pragma region LightComponentHelpers
	void LightComponentHelperBase::OnComponentAdded(IComponent* comp, bool bIsDeserialize)
	{
		Entity entity = comp->OwnerEntity;
		entity.AddComponent<BillboardComponent>();
	}

	void LightComponentHelperBase::OnComponentDestroy(IComponent* comp)
	{
		comp->OwnerEntity.RemoveComponentIfExist<BillboardComponent>();
	}

	void LightComponentHelperBase::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		if (fieldID == GetFieldIDByName<LightComponentBase>("ShadowType"))
		{
			const auto* lightComp = static_cast<LightComponentBase*>(comp);
			lightComp->LightSource->OnShadowTypeChanged();
		}
	}

	void LightComponentHelperBase::PostFieldDeserialize(IComponent* comp, U32 fieldID)
	{
		if (fieldID == GetFieldIDByName<LightComponentBase>("ShadowType"))
		{
			const auto* lightComp = static_cast<LightComponentBase*>(comp);
			lightComp->LightSource->OnShadowTypeChanged();
		}
	}

	void DirectionalLightComponentHelper::OnComponentAdded(IComponent* comp, bool bIsDeserialize)
	{
		LightComponentHelperBase::OnComponentAdded(comp, bIsDeserialize);

		const auto entity = comp->OwnerEntity;
		auto& lightComp = entity.GetComponent<DirectionalLightComponent>();
		auto& billboardComp = entity.GetComponent<BillboardComponent>();
		lightComp.LightSource = CreateRef<DirectionalLight>(entity.GetScene());
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/DirectionalLight.png.zasset");
	}

	void DirectionalLightComponentHelper::OnComponentCopied(IComponent* comp, IComponent* otherComp)
	{
		// Perform the deep copy of smart pointer
		auto* otherLightComp = static_cast<DirectionalLightComponent*>(otherComp);
		auto* lightComp = static_cast<DirectionalLightComponent*>(comp);
		lightComp->LightSource = CreateRef<DirectionalLight>(*otherLightComp->GetDirectionalLight());
	}

	BoxSphereBounds DirectionalLightComponentHelper::GetBounds(IComponent* comp)
	{
		const auto& transformComp = comp->OwnerEntity.GetComponent<TransformComponent>();
		const Sphere sphere{ transformComp.Translation, 0.0f };
		return sphere;
	}

	void PointLightComponentHelper::OnComponentAdded(IComponent* comp, bool bIsDeserialize)
	{
		LightComponentHelperBase::OnComponentAdded(comp, bIsDeserialize);

		const auto entity = comp->OwnerEntity;
		auto& lightComp = entity.GetComponent<PointLightComponent>();
		auto& billboardComp = entity.GetComponent<BillboardComponent>();
		lightComp.LightSource = CreateRef<PointLight>(entity.GetScene());
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/PointLight.png.zasset");
	}

	void PointLightComponentHelper::OnComponentCopied(IComponent* comp, IComponent* otherComp)
	{
		// Perform the deep copy of smart pointer
		auto* otherLightComp = static_cast<PointLightComponent*>(otherComp);
		auto* lightComp = static_cast<PointLightComponent*>(comp);
		lightComp->LightSource = CreateRef<PointLight>(*otherLightComp->GetPointLight());
	}

	void PointLightComponentHelper::PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex)
	{
		LightComponentHelperBase::PostComponentFieldValueEditChange(comp, fieldID, oldValue, elementIndex);

		if (fieldID == GetFieldIDByName<PointLightComponent>("Range"))
		{
			comp->OwnerEntity.UpdateBounds();
		}
	}

	BoxSphereBounds PointLightComponentHelper::GetBounds(IComponent* comp)
	{
		auto* lightComp = static_cast<PointLightComponent*>(comp);
		const auto& transformComp = comp->OwnerEntity.GetComponent<TransformComponent>();
		const float range = lightComp->GetPointLight()->GetRange();
		const Sphere sphere{ transformComp.Translation, range };
		return sphere;
	}

	void SpotLightComponentHelper::OnComponentAdded(IComponent* comp, bool bIsDeserialize)
	{
		LightComponentHelperBase::OnComponentAdded(comp, bIsDeserialize);

		const auto entity = comp->OwnerEntity;
		auto& lightComp = entity.GetComponent<SpotLightComponent>();
		auto& billboardComp = entity.GetComponent<BillboardComponent>();
		lightComp.LightSource = CreateRef<SpotLight>(entity.GetScene());
		billboardComp.TextureAsset = AssetLibrary::LoadAsset<Texture2D>("assets/textures/icons/SpotLight.png.zasset");
	}

	void SpotLightComponentHelper::OnComponentCopied(IComponent* comp, IComponent* otherComp)
	{
		// Perform the deep copy of smart pointer
		auto* otherLightComp = static_cast<SpotLightComponent*>(otherComp);
		auto* lightComp = static_cast<SpotLightComponent*>(comp);
		lightComp->LightSource = CreateRef<SpotLight>(*otherLightComp->GetSpotLight());
	}

	BoxSphereBounds SpotLightComponentHelper::GetBounds(IComponent* comp)
	{
		auto* lightComp = static_cast<SpotLightComponent*>(comp);
		const auto& transformComp = comp->OwnerEntity.GetComponent<TransformComponent>();
		const float range = lightComp->GetSpotLight()->GetRange();
		const Sphere sphere{ transformComp.Translation, range * 0.5f };
		return sphere;
	}

#pragma endregion

}
