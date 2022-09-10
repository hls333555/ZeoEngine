#pragma once

#include "Worlds/EditorPreviewWorldBase.h"

namespace ZeoEngine {

	class AssetView;
	struct MaterialPreviewComponent;
	class Material;
	struct MeshPreviewComponent;
	class Mesh;
	struct TexturePreviewComponent;
	class Texture2D;
	struct ParticleSystemPreviewComponent;
	class ParticleTemplate;

	class AssetPreviewWorldBase : public EditorPreviewWorldBase
	{
	public:
		explicit AssetPreviewWorldBase(std::string worldName);
		virtual ~AssetPreviewWorldBase();

		virtual void OnAttach() override;

		virtual AssetView* GetAssetView() const override { return m_AssetView.get(); }

		void SaveCurrentAsset() const;
		void PostAssetLoad();

	private:
		virtual Ref<Scene> CreateScene() override;
		virtual void PostSceneCreate(const Ref<Scene>& scene) override;

		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) = 0;
		virtual Scope<AssetView> CreateAssetView() = 0;

	private:
		Scope<AssetView> m_AssetView;
	};

	class MaterialPreviewWorld : public AssetPreviewWorldBase
	{
	public:
		using AssetPreviewWorldBase::AssetPreviewWorldBase;

		virtual void LoadAsset(const std::string& path) override;
		virtual void OnAssetSaveAs(const std::string& path) override;

	private:
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;
		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) override;

		virtual Scope<InspectorBase> CreateInspector() override;
		virtual Scope<AssetView> CreateAssetView() override;

		void SetMaterialAsset(MaterialPreviewComponent& materialPreviewComp, Ref<Material> material);
	};

	class MeshPreviewWorld : public AssetPreviewWorldBase
	{
	public:
		explicit MeshPreviewWorld(std::string worldName);

		virtual void LoadAsset(const std::string& path) override;
		virtual void OnAssetSaveAs(const std::string& path) override;

	private:
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;
		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) override;

		virtual Scope<InspectorBase> CreateInspector() override;
		virtual Scope<AssetView> CreateAssetView() override;

		void SetMeshAsset(MeshPreviewComponent& meshPreviewComp, Ref<Mesh> mesh);
	};

	class TexturePreviewWorld : public AssetPreviewWorldBase
	{
	public:
		using AssetPreviewWorldBase::AssetPreviewWorldBase;

		virtual void LoadAsset(const std::string& path) override;
		virtual void OnAssetSaveAs(const std::string& path) override;

	private:
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;
		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) override;

		virtual Scope<InspectorBase> CreateInspector() override;
		virtual Scope<AssetView> CreateAssetView() override;

		void SetTextureAsset(TexturePreviewComponent& texturePreviewComp, Ref<Texture2D> texture);
	};

	class ParticlePreviewWorld : public AssetPreviewWorldBase
	{
	public:
		using AssetPreviewWorldBase::AssetPreviewWorldBase;

		virtual void LoadAsset(const std::string& path) override;
		virtual void OnAssetSaveAs(const std::string& path) override;

	private:
		virtual Ref<Scene> CreateScene() override;
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;
		virtual Entity CreatePreviewEntity(const Ref<Scene>& scene) override;

		virtual Scope<InspectorBase> CreateInspector() override;
		virtual Scope<AssetView> CreateAssetView() override;

		void SetParticleAsset(ParticleSystemPreviewComponent& particlePreviewComp, Ref<ParticleTemplate> particle);
	};
	
}
