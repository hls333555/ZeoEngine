#include "ZEpch.h"
#include "Engine/Renderer/Mesh.h"

#include <assimp/importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Material.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Asset/AssetLibrary.h"

namespace ZeoEngine {

	MeshEntryInstance::MeshEntryInstance(const SceneContext* sceneContext, const MeshEntry& entry, const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo)
		: Drawable(vao, ubo, sceneContext), EntryPtr(&entry)
	{
	}

	Mesh::Mesh(std::string resourcePath)
		: m_MeshResourcePath(std::move(resourcePath))
	{
		Assimp::Importer Importer;
		const aiScene* meshScene = Importer.ReadFile(m_MeshResourcePath.c_str(),
			aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_GlobalScale);
		if (!meshScene)
		{
			ZE_CORE_ERROR("Failed to load mesh! {0}", Importer.GetErrorString());
			return;
		}

		LoadFromMeshScene(meshScene);
	}

	Ref<MeshInstance> Mesh::CreateInstance(const Scene& scene)
	{
		return CreateRef<MeshInstance>(scene.GetContext(), SharedFromThis());
	}

	Ref<Mesh> Mesh::GetDefaultCubeMesh()
	{
		return AssetLibrary::LoadAsset<Mesh>("assets/meshes/Cube.fbx.zasset");
	}

	Ref<Mesh> Mesh::GetDefaultSphereMesh()
	{
		return AssetLibrary::LoadAsset<Mesh>("assets/meshes/Sphere.fbx.zasset");
	}

	Ref<Mesh> Mesh::GetDefaultPlaneMesh()
	{
		return AssetLibrary::LoadAsset<Mesh>("assets/meshes/Plane.fbx.zasset");
	}

	void Mesh::SetDefaultMaterialAsset(U32 index, AssetHandle materialAsset)
	{
		if (index < 0 || index >= m_MaterialAssets.size()) return;

		const AssetHandle oldMaterial = m_MaterialAssets[index];
		if (materialAsset == oldMaterial) return;

		m_MaterialAssets[index] = materialAsset;
	}

	void Mesh::LoadFromMeshScene(const aiScene* meshScene)
	{
		m_VAO = VertexArray::Create();

		LoadMeshEntries(meshScene);

		auto* vertexBuffer = new MeshVertex[m_VertexCount];
		auto* indexBuffer = new U32[m_IndexCount];

		// Load datas before submitting buffers
		LoadDatas(meshScene, vertexBuffer, indexBuffer);

		Ref<VertexBuffer> vbo = VertexBuffer::Create(vertexBuffer, m_VertexCount * sizeof(MeshVertex));
		const BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal"   },
			{ ShaderDataType::Float3, "a_Tangent"  },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};
		vbo->SetLayout(layout);
		m_VAO->AddVertexBuffer(vbo);

		Ref<IndexBuffer> ibo = IndexBuffer::Create(indexBuffer, m_IndexCount);
		m_VAO->SetIndexBuffer(ibo);

		delete[] vertexBuffer;
		delete[] indexBuffer;
	}

	void Mesh::LoadMeshEntries(const aiScene* meshScene)
	{
		// Init material slots with default materials
		m_MaterialAssets.reserve(meshScene->mNumMaterials);
		m_MaterialNames.reserve(meshScene->mNumMaterials);
		for (U32 i = 0; i < meshScene->mNumMaterials; ++i)
		{
			m_MaterialAssets.emplace_back(Material::GetDefaultMaterial()->GetHandle());
			m_MaterialNames.emplace_back(meshScene->mMaterials[i]->GetName().C_Str());
		}

		const U32 meshCount = meshScene->mNumMeshes;
		m_Entries.resize(meshCount);
		for (U32 i = 0; i < meshCount; ++i)
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

	void Mesh::LoadDatas(const aiScene* meshScene, MeshVertex* vertexBuffer, U32* indexBuffer)
	{
		for (SizeT i = 0; i < m_Entries.size(); ++i)
		{
			const aiMesh* mesh = meshScene->mMeshes[i];
			LoadVertexData(mesh, vertexBuffer, m_Entries[i].BaseVertex);
			LoadIndexData(mesh, indexBuffer, m_Entries[i].BaseIndex);
		}
	}

	void Mesh::LoadVertexData(const aiMesh* mesh, MeshVertex* vertexBuffer, U32 baseIndex)
	{
		Box box;
		for (U32 i = 0; i < mesh->mNumVertices; ++i)
		{
			const aiVector3D& position = mesh->mVertices[i];
			const aiVector3D& normal = mesh->mNormals[i];
			const aiVector3D& tangent = mesh->mTangents[i];
			const aiVector3D& texCoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D();
			const Vec3 pos = { position.x, position.y, position.z };
			box += pos;
			vertexBuffer[baseIndex + i].Position = pos;
			vertexBuffer[baseIndex + i].Normal = { normal.x, normal.y, normal.z };
			vertexBuffer[baseIndex + i].Tangent = { tangent.x, tangent.y, tangent.z };
			vertexBuffer[baseIndex + i].TexCoord = { texCoord.x, texCoord.y };
		}
		m_Bounds = m_Bounds + box;
	}

	void Mesh::LoadIndexData(const aiMesh* mesh, U32* indexBuffer, U32 baseIndex)
	{
		for (U32 i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face = mesh->mFaces[i];
			ZE_CORE_ASSERT(face.mNumIndices == 3);

			indexBuffer[baseIndex + i * 3] = face.mIndices[0];
			indexBuffer[baseIndex + i * 3 + 1] = face.mIndices[1];
			indexBuffer[baseIndex + i * 3 + 2] = face.mIndices[2];
		}
	}

	MeshInstance::MeshInstance(const SceneContext* sceneContext, const Ref<Mesh>& mesh)
		: m_MeshPtr(mesh)
	{
		m_ModelUniformBuffer = UniformBuffer::Create(sizeof(ModelData), static_cast<U32>(UniformBufferBinding::Model));
		const auto& entries = m_MeshPtr->GetMeshEntries();
		// Allocate space first so that every element's address remains unchanged
		m_EntryInstances.reserve(entries.size());
		for (const auto& entry : entries)
		{
			m_EntryInstances.emplace_back(sceneContext, entry, mesh->GetVAO(), m_ModelUniformBuffer);
		}
	}

	MeshInstance::MeshInstance(const MeshInstance& other)
		: m_MeshPtr(other.m_MeshPtr)
	{
		m_ModelUniformBuffer = UniformBuffer::Create(sizeof(ModelData), static_cast<U32>(UniformBufferBinding::Model));
		const auto& entries = m_MeshPtr->GetMeshEntries();
		const auto size = entries.size();
		// Allocate space first so that every element's address remains unchanged
		m_EntryInstances.reserve(size);
		for (SizeT i = 0; i < size; ++i)
		{
			const auto& entry = entries[i];
			m_EntryInstances.emplace_back(other.m_EntryInstances[i].GetSceneContext(), entry, m_MeshPtr->GetVAO(), m_ModelUniformBuffer);
		}
	}

	void MeshInstance::Submit(const Mat4& transform, const std::vector<AssetHandle>& materialAssets, I32 entityID)
	{
		m_ModelBuffer.Transform = transform;
		m_ModelBuffer.NormalMatrix = glm::transpose(glm::inverse(transform));
		m_ModelBuffer.EntityID = entityID;
		m_ModelUniformBuffer->SetData(&m_ModelBuffer);

		for (SizeT i = 0; i < m_EntryInstances.size(); ++i)
		{
			auto& entryInstance = m_EntryInstances[i];
			const auto materialIndex = entryInstance.EntryPtr->MaterialIndex;
			entryInstance.Submit(materialAssets[materialIndex]);
		}
	}

}
