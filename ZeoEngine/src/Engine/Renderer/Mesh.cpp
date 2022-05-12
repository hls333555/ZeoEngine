#include "ZEpch.h"
#include "Engine/Renderer/Mesh.h"

#include <assimp/importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Engine/Renderer/VertexArray.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	MeshEntryInstance::MeshEntryInstance(const MeshEntry& entry, AssetHandle<MaterialAsset>& material, const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo, const RenderGraph& renderGraph, bool bIsDeserialize)
		: Drawable(vao, ubo), EntryPtr(&entry), RenderGraphPtr(&renderGraph)
	{
		// This will be done after deserialization if bIsDeserialize is false
		if (!bIsDeserialize)
		{
			BindAndSubmitTechniques(material);
		}
	}

	void MeshEntryInstance::BindAndSubmitTechniques(AssetHandle<MaterialAsset>& material)
	{
		SubmitTechniques(material);
		// Connect callback on new material for this instance
		material->m_OnMaterialInitialized.connect<&MeshEntryInstance::SubmitTechniques>(this);
	}

	void MeshEntryInstance::SubmitTechniques(const AssetHandle<MaterialAsset>& material)
	{
		ClearTechniques();
		for (const auto& technique : material->GetMaterial()->GetRenderTechniques())
		{
			AddTechnique(technique, *RenderGraphPtr);
		}
	}

	Mesh::Mesh(const std::string& path)
	{
		Assimp::Importer Importer;
		const aiScene* meshScene = Importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_GlobalScale);
		if (!meshScene)
		{
			ZE_CORE_ERROR("Failed to load mesh! {0}", Importer.GetErrorString());
			return;
		}

		LoadFromMeshScene(meshScene, path);
	}

	Ref<Mesh> Mesh::Create(const std::string& path)
	{
		class MeshEnableShared : public Mesh
		{
		public:
			explicit MeshEnableShared(const std::string& path)
				: Mesh(path) {}
		};
		return CreateRef<MeshEnableShared>(path);
	}

	void Mesh::LoadFromMeshScene(const aiScene* meshScene, const std::string& path)
	{
		m_VAO = VertexArray::Create();

		LoadMeshEntries(meshScene);

		// TODO: Change to local
		m_VertexBuffer = new MeshVertex[m_VertexCount];
		m_IndexBuffer = new uint32_t[m_IndexCount];

		// Load datas before submitting buffers
		LoadDatas(meshScene);

		Ref<VertexBuffer> vbo = VertexBuffer::Create(m_VertexBuffer, m_VertexCount * sizeof(MeshVertex));
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal"   },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};
		vbo->SetLayout(layout);
		m_VAO->AddVertexBuffer(vbo);

		Ref<IndexBuffer> ibo = IndexBuffer::Create(m_IndexBuffer, m_IndexCount);
		m_VAO->SetIndexBuffer(ibo);

		delete[] m_VertexBuffer;
		delete[] m_IndexBuffer;
	}

	void Mesh::LoadMeshEntries(const aiScene* meshScene)
	{
		// Init material slots with default materials
		m_MaterialSlots.reserve(meshScene->mNumMaterials);
		for (uint32_t i = 0; i < meshScene->mNumMaterials; ++i)
		{
			m_MaterialSlots.emplace_back(MaterialAssetLibrary::GetDefaultMaterialAsset());
		}

		uint32_t meshCount = meshScene->mNumMeshes;
		m_Entries.resize(meshCount);
		for (uint32_t i = 0; i < meshCount; ++i)
		{
			const aiMesh* mesh = meshScene->mMeshes[i];
			m_Entries[i].Name = mesh->mName.C_Str();
			m_Entries[i].BaseVertex = m_VertexCount;
			m_Entries[i].BaseIndex = m_IndexCount;
			m_Entries[i].IndexCount = mesh->mNumFaces * 3;
			m_Entries[i].MaterialIndex = mesh->mMaterialIndex;

			m_VertexCount += mesh->mNumVertices;
			m_IndexCount += m_Entries[i].IndexCount;
		}		
	}

	void Mesh::LoadDatas(const aiScene* meshScene)
	{
		for (size_t i = 0; i < m_Entries.size(); ++i)
		{
			const aiMesh* mesh = meshScene->mMeshes[i];
			LoadVertexData(mesh, m_Entries[i].BaseVertex);
			LoadIndexData(mesh, m_Entries[i].BaseIndex);
		}
	}

	void Mesh::LoadVertexData(const aiMesh* mesh, uint32_t baseIndex)
	{
		Box box;
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
		{
			const aiVector3D& position = mesh->mVertices[i];
			const aiVector3D& normal = mesh->mNormals[i];
			const aiVector3D& texCoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D();
			const glm::vec3 pos = { position.x, position.y, position.z };
			box += pos;
			m_VertexBuffer[baseIndex + i].Position = pos;
			m_VertexBuffer[baseIndex + i].Normal = { normal.x, normal.y, normal.z };
			m_VertexBuffer[baseIndex + i].TexCoord = { texCoord.x, texCoord.y };
		}
		m_Bounds = m_Bounds + box;
	}

	void Mesh::LoadIndexData(const aiMesh* mesh, uint32_t baseIndex)
	{
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face = mesh->mFaces[i];
			ZE_CORE_ASSERT(face.mNumIndices == 3);

			m_IndexBuffer[baseIndex + i * 3] = face.mIndices[0];
			m_IndexBuffer[baseIndex + i * 3 + 1] = face.mIndices[1];
			m_IndexBuffer[baseIndex + i * 3 + 2] = face.mIndices[2];
		}
	}

	MeshInstance::MeshInstance(const Ref<Mesh>& mesh, const RenderGraph& renderGraph, bool bIsDeserialize)
		: m_MeshPtr(mesh)
	{
		m_ModelUniformBuffer = UniformBuffer::Create(sizeof(ModelData), static_cast<uint32_t>(UniformBufferBinding::Model));
		// Copy default materials
		m_Materials = mesh->GetDefaultMaterials();
		const auto& entries = m_MeshPtr->GetMeshEntries();
		// Allocate space first so that every element's address remains unchanged
		m_EntryInstances.reserve(entries.size());
		for (const auto& entry : entries)
		{
			m_EntryInstances.emplace_back(entry, m_Materials[entry.MaterialIndex], mesh->GetVAO(), m_ModelUniformBuffer, renderGraph, bIsDeserialize);
		}
	}

	MeshInstance::MeshInstance(const MeshInstance& other)
		: m_MeshPtr(other.m_MeshPtr), m_Materials(other.m_Materials)
	{
		m_ModelUniformBuffer = UniformBuffer::Create(sizeof(ModelData), static_cast<uint32_t>(UniformBufferBinding::Model));
		const auto& entries = m_MeshPtr->GetMeshEntries();
		const auto size = entries.size();
		// Allocate space first so that every element's address remains unchanged
		m_EntryInstances.reserve(size);
		for (size_t i = 0; i < size; ++i)
		{
			const auto& entry = entries[i];
			m_EntryInstances.emplace_back(entry, m_Materials[entry.MaterialIndex], m_MeshPtr->GetVAO(), m_ModelUniformBuffer, *other.m_EntryInstances[i].RenderGraphPtr);
		}
	}

	void MeshInstance::Create(MeshRendererComponent& meshComp, const RenderGraph& renderGraph, const Ref<MeshInstance>& meshInstanceToCopy, bool bIsDeserialize)
	{
		if (meshInstanceToCopy)
		{
			// Copy construct mesh instance
			meshComp.Instance = CreateRef<MeshInstance>(*meshInstanceToCopy);
		}
		else if (meshComp.Mesh)
		{
			meshComp.Instance = CreateRef<MeshInstance>(meshComp.Mesh->GetMesh(), renderGraph, bIsDeserialize);
		}
	}

	void MeshInstance::SetMaterial(uint32_t index, const AssetHandle<MaterialAsset>& material)
	{
		if (index < 0 || index >= m_Materials.size()) return;

		auto& oldMaterial = m_Materials[index];
		m_Materials[index] = material;
		OnMaterialChanged(index, oldMaterial);
	}

	void MeshInstance::OnMaterialChanged(uint32_t index, AssetHandle<MaterialAsset>& oldMaterial)
	{
		for (auto& entryInstance : m_EntryInstances)
		{
			if (entryInstance.EntryPtr->MaterialIndex == index)
			{
				if (oldMaterial)
				{
					// Disconnect callback on old material for all referenced instances
					oldMaterial->m_OnMaterialInitialized.disconnect<&MeshEntryInstance::SubmitTechniques>(entryInstance);
				}
				SubmitTechniques(entryInstance);
			}
		}
	}

	void MeshInstance::SubmitTechniques(MeshEntryInstance& entryInstance)
	{
		auto& material = m_Materials[entryInstance.EntryPtr->MaterialIndex];
		if (!material) return;

		entryInstance.BindAndSubmitTechniques(material);
	}

	void MeshInstance::SubmitAllTechniques()
	{
		for (auto& entryInstance : m_EntryInstances)
		{
			SubmitTechniques(entryInstance);
		}
	}

	void MeshInstance::Submit(const glm::mat4& transform, int32_t entityID)
	{
		m_ModelBuffer.Transform = transform;
		m_ModelBuffer.NormalMatrix = glm::transpose(glm::inverse(transform));
		m_ModelBuffer.EntityID = entityID;
		m_ModelUniformBuffer->SetData(&m_ModelBuffer);

		for (const auto& entryInstance : m_EntryInstances)
		{
			entryInstance.Submit();
		}
	}

	MeshAsset::MeshAsset(const std::string& path)
		: AssetBase(path)
	{
		Reload(true);
	}

	Ref<MeshAsset> MeshAsset::Create(const std::string& path)
	{
		// A way to allow std::make_shared() to access MeshAsset's private constructor
		class MeshAssetEnableShared : public MeshAsset
		{
		public:
			explicit MeshAssetEnableShared(const std::string& path)
				: MeshAsset(path) {}
		};

		return CreateRef<MeshAssetEnableShared>(path);
	}

	void MeshAsset::Reload(bool bIsCreate)
	{
		const std::string& meshPath = GetID();
		ZE_CORE_ASSERT(PathUtils::DoesPathExist(meshPath));

		m_Mesh = Mesh::Create(meshPath);
		Deserialize(); // Do not call it in constructor if it contains shared_from_this()
	}

	void MeshAsset::Serialize(const std::string& path)
	{
		std::string assetPath = PathUtils::GetNormalizedAssetPath(path);
		if (!PathUtils::DoesPathExist(assetPath)) return;

		SetID(std::move(assetPath));
		ImportableAssetSerializer::Serialize(GetID(), TypeId(), {}); // TODO: Update component instance here
	}

	void MeshAsset::Deserialize()
	{
		if (!PathUtils::DoesPathExist(GetID())) return;

		ImportableAssetSerializer::Deserialize(GetID(), TypeId(), {}, this);  // TODO: Update component instance here
	}

}
