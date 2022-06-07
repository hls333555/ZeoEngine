#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Drawable.h"
#include "Engine/Core/Asset.h"
#include "Engine/Core/AssetLibrary.h"
#include "Engine/Math/BoxSphereBounds.h"

struct aiScene;
struct aiMesh;

namespace ZeoEngine {

	class VertexArray;
	class VertexBuffer;
	class Texture2D;
	class Material;
	class UniformBuffer;
	struct MeshRendererComponent;
	class Scene;

	struct MeshVertex
	{
		Vec3 Position;
		Vec3 Normal;
		Vec2 TexCoord;
	};

	struct MeshEntry
	{
		const char* Name = nullptr; // TODO
		U32 BaseVertex = 0;
		U32 BaseIndex = 0;
		U32 IndexCount = 0;
		U32 MaterialIndex;
	};

	struct MeshEntryInstance : public Drawable
	{
		const MeshEntry* EntryPtr;
		const Weak<Scene> SceneContext;

		MeshEntryInstance(const Weak<Scene>& sceneContext, const MeshEntry& entry, const AssetHandle<Material>& material, const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo, bool bIsDeserialize = false);

		virtual U32 GetBaseVertex() const override { return EntryPtr->BaseVertex; }
		virtual U32 GetBaseIndex() const override { return EntryPtr->BaseIndex; }
		virtual U32 GetIndexCount() const override { return EntryPtr->IndexCount; }

		void BindAndSubmitTechniques(const AssetHandle<Material>& material);
		void SubmitTechniques(const AssetHandle<Material>& material);
	};

	class Mesh : public AssetBase<Mesh>
	{
	public:
		explicit Mesh(const std::string& path);
		static Ref<Mesh> Create(const std::string& path);

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		const Ref<VertexArray>& GetVAO() const { return m_VAO; }
		const auto& GetMeshEntries() const { return m_Entries; }
		U32 GetMeshCount() const { return static_cast<U32>(m_Entries.size()); }

		auto& GetDefaultMaterials() { return m_MaterialSlots; }
		U32 GetMaterialCount() const { return static_cast<U32>(m_MaterialSlots.size()); }

		U32 GetVertexCount() const { return m_VertexCount; }
		U32 GetIndexCount() const { return m_IndexCount; }
		const BoxSphereBounds& GetBounds() const { return m_Bounds; }

	private:
		void LoadFromMeshScene(const aiScene* meshScene);
		void LoadMeshEntries(const aiScene* meshScene);
		void LoadDatas(const aiScene* meshScene, MeshVertex* vertexBuffer, U32* indexBuffer);
		void LoadVertexData(const aiMesh* mesh, MeshVertex* vertexBuffer, U32 baseIndex);
		void LoadIndexData(const aiMesh* mesh, U32* indexBuffer, U32 baseIndex);

	private:
		std::string m_MeshResourcePath;

		Ref<VertexArray> m_VAO;
		U32 m_VertexCount = 0, m_IndexCount = 0;
		std::vector<MeshEntry> m_Entries;
		std::vector<AssetHandle<Material>> m_MaterialSlots;
		BoxSphereBounds m_Bounds;
	};

	REGISTER_ASSET(Mesh,
	Ref<Mesh> operator()(const std::string& path) const
	{
		return Mesh::Create(path);
	},
	static AssetHandle<Mesh> GetDefaultCubeMesh()
	{
		return Get().LoadAsset("assets/editor/meshes/Cube.fbx.zasset");
	}

	static AssetHandle<Mesh> GetDefaultSphereMesh()
	{
		return Get().LoadAsset("assets/editor/meshes/Sphere.fbx.zasset");
	}

	static AssetHandle<Mesh> GetDefaultPlaneMesh()
	{
		return Get().LoadAsset("assets/editor/meshes/Plane.fbx.zasset");
	})

	class MeshInstance
	{
	public:
		MeshInstance(const Ref<Scene>& sceneContext, const AssetHandle<Mesh>& mesh, bool bIsDeserialize);
		MeshInstance(const MeshInstance& other);

		static void Create(const Ref<Scene>& sceneContext, MeshRendererComponent& meshComp, bool bIsDeserialize = false);
		static void Copy(MeshRendererComponent& meshComp, const Ref<MeshInstance>& meshInstanceToCopy);

		const AssetHandle<Mesh>& GetMesh() const { return m_MeshPtr; }
		const auto& GetMeshEntryInstances() const { return m_EntryInstances; }
		auto& GetMeshEntryInstances() { return m_EntryInstances; }
		auto& GetMaterials() { return m_Materials; }
		void SetMaterial(U32 index, const AssetHandle<Material>& material);
		void OnMaterialChanged(U32 index, AssetHandle<Material>& oldMaterial);

		void SubmitTechniques(MeshEntryInstance& entryInstance);
		void SubmitAllTechniques();

		void Submit(const Mat4& transform, I32 entityID);

	private:
		AssetHandle<Mesh> m_MeshPtr;

		struct ModelData
		{
			Mat4 Transform;
			Mat4 NormalMatrix;

			// Editor-only
			I32 EntityID;
		};
		ModelData m_ModelBuffer;
		Ref<UniformBuffer> m_ModelUniformBuffer;
		std::vector<MeshEntryInstance> m_EntryInstances;
		std::vector<AssetHandle<Material>> m_Materials;
	};
}
