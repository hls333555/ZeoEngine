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
	class RenderGraph;

	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
	};

	struct MeshEntry
	{
		const char* Name = nullptr; // TODO
		uint32_t BaseVertex = 0;
		uint32_t BaseIndex = 0;
		uint32_t IndexCount = 0;
		uint32_t MaterialIndex;
	};

	struct MeshEntryInstance : public Drawable
	{
		const MeshEntry* EntryPtr;
		const RenderGraph* RenderGraphPtr;

		MeshEntryInstance(const MeshEntry& entry, AssetHandle<Material>& material, const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo, const RenderGraph& renderGraph, bool bIsDeserialize = false);

		virtual uint32_t GetBaseVertex() const override { return EntryPtr->BaseVertex; }
		virtual uint32_t GetBaseIndex() const override { return EntryPtr->BaseIndex; }
		virtual uint32_t GetIndexCount() const override { return EntryPtr->IndexCount; }

		void BindAndSubmitTechniques(AssetHandle<Material>& material);
		void SubmitTechniques(const AssetHandle<Material>& material);
	};

	class Mesh
	{
	private:
		explicit Mesh(const std::string& path);
	public:
		static Ref<Mesh> Create(const std::string& path);

		const Ref<VertexArray>& GetVAO() const { return m_VAO; }
		const auto& GetMeshEntries() const { return m_Entries; }
		uint32_t GetMeshCount() const { return static_cast<uint32_t>(m_Entries.size()); }

		auto& GetDefaultMaterials() { return m_MaterialSlots; }
		uint32_t GetMaterialCount() const { return static_cast<uint32_t>(m_MaterialSlots.size()); }

		uint32_t GetVertexCount() const { return m_VertexCount; }
		uint32_t GetIndexCount() const { return m_IndexCount; }
		const BoxSphereBounds& GetBounds() const { return m_Bounds; }

	private:
		void LoadFromMeshScene(const aiScene* meshScene, const std::string& path);
		void LoadMeshEntries(const aiScene* meshScene);
		void LoadDatas(const aiScene* meshScene);
		void LoadVertexData(const aiMesh* mesh, uint32_t baseIndex);
		void LoadIndexData(const aiMesh* mesh, uint32_t baseIndex);

	private:
		Ref<VertexArray> m_VAO;
		uint32_t m_VertexCount = 0, m_IndexCount = 0;
		MeshVertex* m_VertexBuffer = nullptr;
		uint32_t* m_IndexBuffer = nullptr;
		std::vector<MeshEntry> m_Entries;
		std::vector<AssetHandle<Material>> m_MaterialSlots;
		BoxSphereBounds m_Bounds;
	};

	class MeshInstance
	{
	public:
		MeshInstance(const Ref<Mesh>& mesh, const RenderGraph& renderGraph, bool bIsDeserialize);
		MeshInstance(const MeshInstance& other);

		static void Create(MeshRendererComponent& meshComp, const RenderGraph& renderGraph, const Ref<MeshInstance>& meshInstanceToCopy = nullptr, bool bIsDeserialize = false);

		const Ref<Mesh>& GetMesh() const { return m_MeshPtr; }
		const auto& GetMeshEntryInstances() const { return m_EntryInstances; }
		auto& GetMeshEntryInstances() { return m_EntryInstances; }
		auto& GetMaterials() { return m_Materials; }
		void SetMaterial(uint32_t index, const AssetHandle<Material>& material);
		void OnMaterialChanged(uint32_t index, AssetHandle<Material>& oldMaterial);

		void SubmitTechniques(MeshEntryInstance& entryInstance);
		void SubmitAllTechniques();

		void Submit(const glm::mat4& transform, int32_t entityID);

	private:
		Ref<Mesh> m_MeshPtr;

		struct ModelData
		{
			glm::mat4 Transform;
			glm::mat4 NormalMatrix;

			// Editor-only
			int32_t EntityID;
		};
		ModelData m_ModelBuffer;
		Ref<UniformBuffer> m_ModelUniformBuffer;
		std::vector<MeshEntryInstance> m_EntryInstances;
		std::vector<AssetHandle<Material>> m_Materials;
	};

	class MeshAsset : public AssetBase<MeshAsset>
	{
	private:
		explicit MeshAsset(const std::string& path);

	public:
		static Ref<MeshAsset> Create(const std::string& path);

		const Ref<Mesh>& GetMesh() const { return m_Mesh; }

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		virtual void Reload(bool bIsCreate) override;

	private:
		Ref<Mesh> m_Mesh;
	};

	struct MeshAssetLoader final
	{
		using result_type = Ref<MeshAsset>;

		// TODO:
		Ref<MeshAsset> operator()(const std::string& path, bool bIsReload) const
		{
			return MeshAsset::Create(path);
		}
	};

	class MeshAssetLibrary : public AssetLibrary<MeshAssetLibrary, MeshAsset, MeshAssetLoader>
	{
	public:
		static AssetHandle<MeshAsset> GetDefaultCubeMesh()
		{
			return Get().LoadAsset("assets/editor/meshes/Cube.fbx.zasset");
		}

		static AssetHandle<MeshAsset> GetDefaultSphereMesh()
		{
			return Get().LoadAsset("assets/editor/meshes/Sphere.fbx.zasset");
		}

		static AssetHandle<MeshAsset> GetDefaultPlaneMesh()
		{
			return Get().LoadAsset("assets/editor/meshes/Plane.fbx.zasset");
		}
	};
}
