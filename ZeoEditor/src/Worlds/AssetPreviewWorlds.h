#pragma once

#include "Worlds/EditorPreviewWorldBase.h"

namespace ZeoEngine {

	class AssetView;
	struct MaterialDetailComponent;
	class Material;
	struct MeshDetailComponent;
	class Mesh;
	struct TextureDetailComponent;
	class Texture2D;
	struct ParticleSystemDetailComponent;
	class ParticleTemplate;

	class AssetPreviewWorldBase : public EditorPreviewWorldBase
	{
	public:
		explicit AssetPreviewWorldBase(std::string worldName);
		virtual ~AssetPreviewWorldBase();

		virtual void OnAttach() override;
		virtual void PostAttach() override;

		virtual AssetView* GetAssetView() const override { return m_AssetView.get(); }

		void SaveCurrentAsset() const;

	private:
		virtual Entity CreatePreviewEntity() = 0;
		virtual Scope<AssetView> CreateAssetView() = 0;

	private:
		Scope<AssetView> m_AssetView;
	};

	class MaterialPreviewWorld : public AssetPreviewWorldBase
	{
	public:
		using AssetPreviewWorldBase::AssetPreviewWorldBase;

		virtual void OnAttach() override;

	private:
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;
		virtual Entity CreatePreviewEntity() override;

		virtual Ref<IAsset> LoadAssetImpl(const std::string& path, bool bForce) override;

		virtual Scope<InspectorBase> CreateInspector() override;
		virtual Scope<AssetView> CreateAssetView() override;
	};

	class MeshPreviewWorld : public AssetPreviewWorldBase
	{
	public:
		using AssetPreviewWorldBase::AssetPreviewWorldBase;

		virtual void OnAttach() override;

	private:
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;
		virtual Entity CreatePreviewEntity() override;

		virtual Ref<IAsset> LoadAssetImpl(const std::string& path, bool bForce) override;

		virtual Scope<InspectorBase> CreateInspector() override;
		virtual Scope<AssetView> CreateAssetView() override;
	};

	class TexturePreviewWorld : public AssetPreviewWorldBase
	{
	public:
		using AssetPreviewWorldBase::AssetPreviewWorldBase;

		virtual void OnAttach() override;

	private:
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;
		virtual Entity CreatePreviewEntity() override;

		virtual Ref<IAsset> LoadAssetImpl(const std::string& path, bool bForce) override;

		virtual Scope<InspectorBase> CreateInspector() override;
		virtual Scope<AssetView> CreateAssetView() override;
	};

	class ParticlePreviewWorld : public AssetPreviewWorldBase
	{
	public:
		using AssetPreviewWorldBase::AssetPreviewWorldBase;

		virtual void OnAttach() override;

	private:
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;
		virtual Entity CreatePreviewEntity() override;

		virtual Ref<IAsset> LoadAssetImpl(const std::string& path, bool bForce) override;

		virtual Scope<InspectorBase> CreateInspector() override;
		virtual Scope<AssetView> CreateAssetView() override;
	};

	class PhysicsMaterialPreviewWorld : public AssetPreviewWorldBase
	{
	public:
		using AssetPreviewWorldBase::AssetPreviewWorldBase;

	private:
		virtual Ref<SceneRenderer> CreateSceneRenderer() override;
		virtual Entity CreatePreviewEntity() override;

		virtual Ref<IAsset> LoadAssetImpl(const std::string& path, bool bForce) override;

		virtual Scope<InspectorBase> CreateInspector() override;
		virtual Scope<AssetView> CreateAssetView() override;
	};
	
}
