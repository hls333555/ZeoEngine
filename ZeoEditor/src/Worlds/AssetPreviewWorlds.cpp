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

	void AssetPreviewWorldBase::PostSceneCreate(const Ref<Scene>& scene)
	{
		const Entity entity = CreatePreviewEntity(*scene);
		SetContextEntity(entity);
	}

	void AssetPreviewWorldBase::OnAttach()
	{
		EditorPreviewWorldBase::OnAttach();

		m_AssetView = CreateAssetView();
	}

	void AssetPreviewWorldBase::SaveCurrentAsset() const
	{
		const AssetHandle handle = GetAsset()->GetHandle();
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(handle);
		const auto& assetPath = metadata->Path;
		SaveAsset(assetPath);
	}

#pragma region MaterialPreviewWorld
	Scope<SceneObserverSystemBase> MaterialPreviewWorld::CreateSceneObserverSystem()
	{
		return CreateScope<MaterialPreviewObserverSystem>();
	}

	Ref<SceneRenderer> MaterialPreviewWorld::CreateSceneRenderer()
	{
		return CreateRef<MaterialPreviewSceneRenderer>();
	}

	Entity MaterialPreviewWorld::CreatePreviewEntity(Scene& scene)
	{
		Entity previewMaterialEntity = scene.CreateEntity("Preview Material");
		previewMaterialEntity.AddComponent<MaterialDetailComponent>();
		previewMaterialEntity.AddComponent<MeshRendererComponent>(Mesh::GetDefaultSphereMesh()->GetHandle());
		previewMaterialEntity.AddComponent<DirectionalLightComponent>();
		return previewMaterialEntity;
	}

	Ref<IAsset> MaterialPreviewWorld::LoadAssetImpl(const std::string& path, bool bForce)
	{
		auto& materialComp = GetContextEntity().GetComponent<MaterialDetailComponent>();
		Ref<IAsset> material = materialComp.LoadedMaterial = AssetLibrary::LoadAsset<Material>(path, bForce);
		auto& meshComp = GetContextEntity().GetComponent<MeshRendererComponent>();
		meshComp.MaterialAssets[0] = material->GetHandle();
		return material;
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
	}

	Scope<SceneObserverSystemBase> MeshPreviewWorld::CreateSceneObserverSystem()
	{
		return CreateScope<MeshPreviewObserverSystem>();
	}

	Ref<SceneRenderer> MeshPreviewWorld::CreateSceneRenderer()
	{
		return CreateRef<MeshPreviewSceneRenderer>();
	}

	Entity MeshPreviewWorld::CreatePreviewEntity(Scene& scene)
	{
		Entity previewMeshEntity = scene.CreateEntity("Preview Mesh");
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
	Scope<SceneObserverSystemBase> TexturePreviewWorld::CreateSceneObserverSystem()
	{
		return CreateScope<TexturePreviewObserverSystem>();
	}

	Ref<SceneRenderer> TexturePreviewWorld::CreateSceneRenderer()
	{
		return nullptr;
	}

	Entity TexturePreviewWorld::CreatePreviewEntity(Scene& scene)
	{
		Entity previewTextureEntity = scene.CreateEntity("Preview Texture");
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
		RegisterSystem<ParticlePreviewUpdateSystem>(this);

		AssetPreviewWorldBase::OnAttach();
	}

	Scope<SceneObserverSystemBase> ParticlePreviewWorld::CreateSceneObserverSystem()
	{
		ZE_CORE_ASSERT(false);
		return {};
	}

	Ref<SceneRenderer> ParticlePreviewWorld::CreateSceneRenderer()
	{
		return CreateRef<ParticlePreviewSceneRenderer>();
	}

	Entity ParticlePreviewWorld::CreatePreviewEntity(Scene& scene)
	{
		Entity previewParticleEntity = scene.CreateEntity("Preview Particle");
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

}
