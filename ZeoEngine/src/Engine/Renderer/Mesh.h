#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Core/Asset.h"
#include "Engine/Core/AssetLibrary.h"

struct aiScene;
struct aiMesh;

namespace ZeoEngine {

	class VertexArray;
	class VertexBuffer;
	class Texture2D;
	class MaterialAsset;

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
	};

	struct MeshEntry
	{
		uint32_t VertexBufferPtr = 0;
		uint32_t IndexBufferPtr = 0;
		uint32_t IndexCount = 0;
		uint32_t MaterialIndex = 0;
	};

	class Mesh
	{
	private:
		explicit Mesh(const std::string& path);
	public:
		static Ref<Mesh> Create(const std::string& path);

		const Ref<VertexArray>& GetVAO() const { return m_VAO; }

		const MeshEntry* GetMeshEntries() const { return m_Entries.data(); }
		uint32_t GetMeshCount() const { return static_cast<uint32_t>(m_Entries.size()); }

		std::vector<AssetHandle<MaterialAsset>>& GetMaterials() { return m_MaterialSlots; }
		void Mesh::SetMaterial(uint32_t index, const AssetHandle<MaterialAsset>& material)
		{
			if (index < 0 || index >= m_MaterialSlots.size()) return;

			m_MaterialSlots[index] = material;
		}
		uint32_t GetMaterialCount() const { return static_cast<uint32_t>(m_MaterialSlots.size()); }

	private:
		void LoadFromMeshScene(const aiScene* meshScene, const std::string& path);
		void LoadMeshEntries(const aiScene* meshScene);
		void LoadDatas(const aiScene* meshScene);
		void LoadVertexData(const aiMesh* mesh, uint32_t baseIndex);
		void LoadIndexData(const aiMesh* mesh, uint32_t baseIndex);

	private:
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		uint32_t m_VertexCount = 0, m_IndexCount = 0;
		Vertex* m_VertexBuffer = nullptr;
		uint32_t* m_IndexBuffer = nullptr;
		std::vector<MeshEntry> m_Entries;

		std::vector<AssetHandle<MaterialAsset>> m_MaterialSlots;
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

		virtual void Reload() override;

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
		static AssetHandle<MeshAsset> GetDefaultSphereMesh()
		{
			return MeshAssetLibrary::Get().LoadAsset("assets/editor/meshes/Sphere.fbx.zasset");
		}
	};
}
