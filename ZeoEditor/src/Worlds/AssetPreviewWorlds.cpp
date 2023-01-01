#include "Worlds/AssetPreviewWorlds.h"

#include "Core/Editor.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Systems.h"
#include "Inspectors/MaterialInspector.h"
#include "SceneRenderers/AssetPreviewSceneRenderers.h"
#include "Panels/AssetViews.h"

namespace ZeoEngine {

	AssetPreviewWorldBase::AssetPreviewWorldBase(std::string worldName)
		: EditorPreviewWorldBase(std::move(worldName))
	{
	}

	AssetPreviewWorldBase::~AssetPreviewWorldBase() = default;

	void AssetPreviewWorldBase::OnAttach()
	{
		EditorPreviewWorldBase::OnAttach();

		m_AssetView = CreateAssetView();
	}

	void AssetPreviewWorldBase::PostAttach()
	{
		// Create preview entity and add various components after systems all begin registered
		// so that component added event can be triggered properly
		const Entity entity = CreatePreviewEntity();
		SetContextEntity(entity);
	}

	void AssetPreviewWorldBase::SaveCurrentAsset() const
	{
		const AssetHandle handle = GetAsset()->GetHandle();
		const auto* metadata = AssetRegistry::Get().GetAssetMetadata(handle);
		const auto& assetPath = metadata->Path;
		SaveAsset(assetPath);
	}

#pragma region MaterialPreviewWorld
	void MaterialPreviewWorld::OnAttach()
	{
		AssetPreviewWorldBase::OnAttach();

		RegisterSystem<BoundsSystem>(this);
		RegisterSystem<MaterialPreviewSystem>(this);
		RegisterSystem<MeshSystem>(this);
		RegisterSystem<DirectionalLightSystem>(this);
	}

	Ref<SceneRenderer> MaterialPreviewWorld::CreateSceneRenderer()
	{
		return CreateRef<MaterialPreviewSceneRenderer>();
	}

	Entity MaterialPreviewWorld::CreatePreviewEntity()
	{
		Entity previewMaterialEntity = GetActiveScene()->CreateEntity("Preview Material");
		previewMaterialEntity.AddComponent<MaterialDetailComponent>();
		previewMaterialEntity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultSphereMesh()->GetHandle());
		previewMaterialEntity.AddComponent<DirectionalLightComponent>();
		return previewMaterialEntity;
	}

	Ref<IAsset> MaterialPreviewWorld::LoadAssetImpl(const std::string& path, bool bForce)
	{
		auto& materialComp = GetContextEntity().GetComponent<MaterialDetailComponent>();
		materialComp.LoadedMaterial = AssetLibrary::LoadAsset<Material>(path, bForce);
		auto& meshComp = GetContextEntity().GetComponent<MeshRendererComponent>();
		meshComp.MaterialAssets[0] = materialComp.LoadedMaterial->GetHandle();
		return materialComp.LoadedMaterial;
	}

	Scope<InspectorBase> MaterialPreviewWorld::CreateInspector()
	{
		return CreateScope<MaterialInspector>(this, entt::type_hash<MaterialDetailComponent>::value());
	}

	Scope<AssetView> MaterialPreviewWorld::CreateAssetView()
	{
		return CreateScope<AssetView>();
	}
#pragma endregion

#pragma region MeshPreviewWorld
	void MeshPreviewWorld::OnAttach()
	{
		AssetPreviewWorldBase::OnAttach();

		GetEditorCamera().SetFarClip(10000.0f);

		RegisterSystem<BoundsSystem>(this);
		RegisterSystem<MeshPreviewSystem>(this);
		RegisterSystem<DirectionalLightSystem>(this);
	}

	Ref<SceneRenderer> MeshPreviewWorld::CreateSceneRenderer()
	{
		return CreateRef<MeshPreviewSceneRenderer>();
	}

	Entity MeshPreviewWorld::CreatePreviewEntity()
	{
		Entity previewMeshEntity = GetActiveScene()->CreateEntity("Preview Mesh");
		previewMeshEntity.AddComponent<MeshDetailComponent>();
		previewMeshEntity.AddComponent<DirectionalLightComponent>();
		return previewMeshEntity;
	}

	Ref<IAsset> MeshPreviewWorld::LoadAssetImpl(const std::string& path, bool bForce)
	{
		Ref<IAsset> mesh;
		GetContextEntity().PatchComponentSingleField<MeshDetailComponent>("LoadedMesh"_hs, [&path, bForce, &mesh](MeshDetailComponent& meshComp)
		{
			mesh = meshComp.LoadedMesh = AssetLibrary::LoadAsset<Mesh>(path, bForce);
		});
		return mesh;
	}

	Scope<InspectorBase> MeshPreviewWorld::CreateInspector()
	{
		return CreateScope<AssetInspector>(this, entt::type_hash<MeshDetailComponent>::value());
	}

	Scope<AssetView> MeshPreviewWorld::CreateAssetView()
	{
		return CreateScope<AssetView>();
	}
#pragma endregion

#pragma region TexturePreviewWorld
	void TexturePreviewWorld::OnAttach()
	{
		AssetPreviewWorldBase::OnAttach();

		RegisterSystem<TexturePreviewSystem>(this);
	}

	Ref<SceneRenderer> TexturePreviewWorld::CreateSceneRenderer()
	{
		return nullptr;
	}

	Entity TexturePreviewWorld::CreatePreviewEntity()
	{
		Entity previewTextureEntity = GetActiveScene()->CreateEntity("Preview Texture");
		previewTextureEntity.AddComponent<TextureDetailComponent>();
		return previewTextureEntity;
	}

	Ref<IAsset> TexturePreviewWorld::LoadAssetImpl(const std::string& path, bool bForce)
	{
		auto& textureComp = GetContextEntity().GetComponent<TextureDetailComponent>();
		textureComp.LoadedTexture = AssetLibrary::LoadAsset<Texture2D>(path, bForce);
		return textureComp.LoadedTexture;
	}

	Scope<InspectorBase> TexturePreviewWorld::CreateInspector()
	{
		return CreateScope<AssetInspector>(this, entt::type_hash<TextureDetailComponent>::value());
	}

	Scope<AssetView> TexturePreviewWorld::CreateAssetView()
	{
		return CreateScope<TextureAssetView>(this);
	}
#pragma endregion

#pragma region ParticlePreviewWorld
	void ParticlePreviewWorld::OnAttach()
	{
		AssetPreviewWorldBase::OnAttach();

		RegisterSystem<ParticlePreviewUpdateSystem>(this);
	}

	Ref<SceneRenderer> ParticlePreviewWorld::CreateSceneRenderer()
	{
		return CreateRef<ParticlePreviewSceneRenderer>();
	}

	Entity ParticlePreviewWorld::CreatePreviewEntity()
	{
		Entity previewParticleEntity = GetActiveScene()->CreateEntity("Preview Particle");
		auto& particlePreviewComp = previewParticleEntity.AddComponent<ParticleSystemDetailComponent>();
		// TODO:
		// For loading phase, instance will be created in ParticleScene::PostLoad
		ParticleSystemInstance::Create(particlePreviewComp);
		return previewParticleEntity;
	}

	Ref<IAsset> ParticlePreviewWorld::LoadAssetImpl(const std::string& path, bool bForce)
	{
		Ref<IAsset> particle;
		GetContextEntity().PatchComponent<ParticleSystemDetailComponent>([&path, bForce, &particle](ParticleSystemDetailComponent& particleComp)
		{
			particle = particleComp.ParticleTemplateAsset = AssetLibrary::LoadAsset<ParticleTemplate>(path, bForce);
		});
		return particle;
	}

	Scope<InspectorBase> ParticlePreviewWorld::CreateInspector()
	{
		return CreateScope<AssetInspector>(this, entt::type_hash<ParticleSystemDetailComponent>::value());
	}

	Scope<AssetView> ParticlePreviewWorld::CreateAssetView()
	{
		return CreateScope<AssetView>();
	}
#pragma endregion

#pragma region PhysicsMaterialPreviewWorld
	Ref<SceneRenderer> PhysicsMaterialPreviewWorld::CreateSceneRenderer()
	{
		return nullptr;
	}

	Entity PhysicsMaterialPreviewWorld::CreatePreviewEntity()
	{
		Entity previewPhysicsMaterialEntity = GetActiveScene()->CreateEntity("Preview Physics Material");
		previewPhysicsMaterialEntity.AddComponent<PhysicsMaterialDetailComponent>();
		return previewPhysicsMaterialEntity;
	}

	Ref<IAsset> PhysicsMaterialPreviewWorld::LoadAssetImpl(const std::string& path, bool bForce)
	{
		auto& physicsMaterialComp = GetContextEntity().GetComponent<PhysicsMaterialDetailComponent>();
		physicsMaterialComp.LoadedPhysicsMaterial = AssetLibrary::LoadAsset<PhysicsMaterial>(path, bForce);
		return physicsMaterialComp.LoadedPhysicsMaterial;
	}

	Scope<InspectorBase> PhysicsMaterialPreviewWorld::CreateInspector()
	{
		return CreateScope<AssetInspector>(this, entt::type_hash<PhysicsMaterialDetailComponent>::value());
	}

	Scope<AssetView> PhysicsMaterialPreviewWorld::CreateAssetView()
	{
		return nullptr;
	}
#pragma endregion

}
