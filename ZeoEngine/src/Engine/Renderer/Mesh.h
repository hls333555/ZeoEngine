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
	class MaterialAsset;
	class UniformBuffer;

	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
	};

	struct MeshEntry : public Drawable
	{
		const char* Name = nullptr;
		uint32_t BaseVertex = 0;
		uint32_t BaseIndex = 0;
		AssetHandle<MaterialAsset>* MaterialPtr = nullptr;

		virtual uint32_t GetBaseVertex() const override { return BaseVertex; }
		virtual uint32_t GetBaseIndex() const override { return BaseIndex; }
		virtual void Submit() const override;

		void SetVertexArray(const Ref<VertexArray>& vao) { m_VAO = vao; }
		void SetIndexCount(uint32_t count) { m_IndexCount = count; }
		void SetModelUniformBuffer(const Ref<UniformBuffer>& ubo) { m_ModelUniformBuffer = ubo; }
	};

	class Mesh
	{
	private:
		explicit Mesh(const std::string& path);
	public:
		static Ref<Mesh> Create(const std::string& path);

		const MeshEntry* GetMeshEntries() const { return m_Entries.data(); }
		uint32_t GetMeshCount() const { return static_cast<uint32_t>(m_Entries.size()); }

		auto& GetMaterials() { return m_MaterialSlots; }
		void Mesh::SetMaterial(uint32_t index, const AssetHandle<MaterialAsset>& material)
		{
			if (index < 0 || index >= m_MaterialSlots.size()) return;

			m_MaterialSlots[index] = material;
		}
		uint32_t GetMaterialCount() const { return static_cast<uint32_t>(m_MaterialSlots.size()); }

		uint32_t GetVertexCount() const { return m_VertexCount; }
		uint32_t GetIndexCount() const { return m_IndexCount; }
		const BoxSphereBounds& GetBounds() const { return m_Bounds; }

		void Submit(const glm::mat4& transform, int32_t entityID);

	private:
		void LoadFromMeshScene(const aiScene* meshScene, const std::string& path);
		void LoadMeshEntries(const aiScene* meshScene, const Ref<VertexArray>& vao);
		void LoadDatas(const aiScene* meshScene);
		void LoadVertexData(const aiMesh* mesh, uint32_t baseIndex);
		void LoadIndexData(const aiMesh* mesh, uint32_t baseIndex);

	private:
		uint32_t m_VertexCount = 0, m_IndexCount = 0;
		MeshVertex* m_VertexBuffer = nullptr;
		uint32_t* m_IndexBuffer = nullptr;
		std::vector<MeshEntry> m_Entries;
		std::vector<AssetHandle<MaterialAsset>> m_MaterialSlots;
		BoxSphereBounds m_Bounds;

		struct ModelData
		{
			glm::mat4 Transform;
			glm::mat4 NormalMatrix;

			// Editor-only
			int32_t EntityID;
		};
		ModelData m_ModelBuffer;
		// TODO: Move to MeshInstance
		Ref<UniformBuffer> m_ModelUniformBuffer;
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

	struct MeshAssetLoader final : AssetLoader<MeshAssetLoader, MeshAsset>
	{
		AssetHandle<MeshAsset> load(const std::string& path) const
		{
			return MeshAsset::Create(path);
		}
	};

	class MeshAssetLibrary : public AssetLibrary<MeshAssetLibrary, MeshAsset, MeshAssetLoader>
	{
	public:
		static AssetHandle<MeshAsset> GetDefaultCubeMesh()
		{
			return MeshAssetLibrary::Get().LoadAsset("assets/editor/meshes/Cube.fbx.zasset");
		}

		static AssetHandle<MeshAsset> GetDefaultSphereMesh()
		{
			return MeshAssetLibrary::Get().LoadAsset("assets/editor/meshes/Sphere.fbx.zasset");
		}

		static AssetHandle<MeshAsset> GetDefaultPlaneMesh()
		{
			return MeshAssetLibrary::Get().LoadAsset("assets/editor/meshes/Plane.fbx.zasset");
		}
	};
}
