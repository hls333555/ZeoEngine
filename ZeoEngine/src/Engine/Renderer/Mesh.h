#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Drawable.h"
#include "Engine/Asset/Asset.h"
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
	struct SceneContext;

	struct MeshVertex
	{
		Vec3 Position;
		Vec3 Normal;
		Vec3 Tangent;
		Vec2 TexCoord;
	};

	struct MeshEntry
	{
		std::string Name;
		U32 BaseVertex = 0;
		U32 BaseIndex = 0;
		U32 IndexCount = 0;
		U32 MaterialIndex;
	};

	struct MeshEntryInstance : public Drawable
	{
		const MeshEntry* EntryPtr;

		MeshEntryInstance(const SceneContext* sceneContext, const MeshEntry& entry, const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo);

		virtual U32 GetBaseVertex() const override { return EntryPtr->BaseVertex; }
		virtual U32 GetBaseIndex() const override { return EntryPtr->BaseIndex; }
		virtual U32 GetIndexCount() const override { return EntryPtr->IndexCount; }
	};

	class Mesh : public AssetBase<Mesh>
	{
	public:
		explicit Mesh(std::string resourcePath);

		[[nodiscard]] Ref<class MeshInstance> CreateInstance(const Scene& scene);

		// All these built-in meshes have an extent of 1m
		// To export meshes from blender with correct scale and rotation, see https://www.immersivelimit.com/tutorials/blender-to-unity-export-correct-scale-rotation#:~:text=By%20default%2C%20Blender%20exports%20.,89.98%20and%20scale%20of%20100
		static Ref<Mesh> GetDefaultCubeMesh();
		static Ref<Mesh> GetDefaultSphereMesh();
		static Ref<Mesh> GetDefaultCapsuleMesh();
		static Ref<Mesh> GetDefaultPlaneMesh();

		const Ref<VertexArray>& GetVAO() const { return m_VAO; }
		const auto& GetMeshEntries() const { return m_Entries; }
		U32 GetMeshCount() const { return static_cast<U32>(m_Entries.size()); }

		auto& GetDefaultMaterialAssets() { return m_MaterialAssets; }
		void SetDefaultMaterialAsset(U32 index, AssetHandle materialAsset);
		const auto& GetMaterialNames() const { return m_MaterialNames; }
		U32 GetMaterialCount() const { return static_cast<U32>(m_MaterialAssets.size()); }

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
		std::vector<AssetHandle> m_MaterialAssets;
		std::vector<std::string> m_MaterialNames;
		BoxSphereBounds m_Bounds;
	};

	class MeshInstance
	{
	public:
		MeshInstance(const SceneContext* sceneContext, const Ref<Mesh>& mesh);
		MeshInstance(const MeshInstance& other);

		const Ref<Mesh>& GetMesh() const { return m_MeshPtr; }

		void Submit(const Mat4& transform, const std::vector<AssetHandle>& materialAssets, I32 entityID);

	private:
		Ref<Mesh> m_MeshPtr;

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
	};

}
