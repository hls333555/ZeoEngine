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
	class Scene;

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
		const Weak<Scene> SceneContext;
		const Ref<Material> MaterialRef;

		MeshEntryInstance(const Weak<Scene>& sceneContext, const MeshEntry& entry, const Ref<Material>& material, const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo, bool bIsDeserialize = false);
		MeshEntryInstance(MeshEntryInstance&&) = default; // Once destructor is defined, we must also define a move constructor in order for it to be used instead of the copy constructor (which is deleted in the base class)
		virtual ~MeshEntryInstance();

		virtual U32 GetBaseVertex() const override { return EntryPtr->BaseVertex; }
		virtual U32 GetBaseIndex() const override { return EntryPtr->BaseIndex; }
		virtual U32 GetIndexCount() const override { return EntryPtr->IndexCount; }

		void BindAndSubmitTechniques(const Ref<Material>& material);
		void SubmitTechniques(const Ref<Material>& material);
	};

	class Mesh : public AssetBase<Mesh>
	{
	public:
		explicit Mesh(std::string resourcePath);

		static Ref<Mesh> GetDefaultCubeMesh();
		static Ref<Mesh> GetDefaultSphereMesh();
		static Ref<Mesh> GetDefaultPlaneMesh();

		const Ref<VertexArray>& GetVAO() const { return m_VAO; }
		const auto& GetMeshEntries() const { return m_Entries; }
		U32 GetMeshCount() const { return static_cast<U32>(m_Entries.size()); }

		auto& GetDefaultMaterials() { return m_MaterialSlots; }
		const auto& GetMaterialNames() const { return m_MaterialNames; }
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
		std::vector<Ref<Material>> m_MaterialSlots;
		std::vector<std::string> m_MaterialNames;
		BoxSphereBounds m_Bounds;
	};

	class MeshInstance
	{
	public:
		MeshInstance(const Ref<Scene>& sceneContext, const Ref<Mesh>& mesh, bool bIsDeserialize);
		MeshInstance(const MeshInstance& other);

		static void Create(const Ref<Scene>& sceneContext, MeshRendererComponent& meshComp, bool bIsDeserialize = false);
		static void Copy(MeshRendererComponent& meshComp, const Ref<MeshInstance>& meshInstanceToCopy);

		const Ref<Mesh>& GetMesh() const { return m_MeshPtr; }
		const auto& GetMeshEntryInstances() const { return m_EntryInstances; }
		auto& GetMeshEntryInstances() { return m_EntryInstances; }
		auto& GetMaterials() { return m_Materials; }
		void SetMaterial(U32 index, const Ref<Material>& material);
		void OnMaterialChanged(U32 index, Ref<Material>& oldMaterial);

		void SubmitTechniques(MeshEntryInstance& entryInstance);
		void SubmitAllTechniques();

		void Submit(const Mat4& transform, I32 entityID);

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
		std::vector<Ref<Material>> m_Materials;
	};
}
