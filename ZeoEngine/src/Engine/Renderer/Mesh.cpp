#include "ZEpch.h"
#include "Engine/Renderer/Mesh.h"

#include <assimp/importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	Mesh::Mesh(const std::string& path)
	{
		Assimp::Importer Importer;
		const aiScene* meshScene = Importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
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
		LoadMeshEntries(meshScene);

		m_VertexBuffer = new Vertex[m_VertexCount];
		m_IndexBuffer = new uint32_t[m_IndexCount];

		// Load datas before submitting buffers
		LoadDatas(meshScene);

		m_VAO = VertexArray::Create();

		m_VBO = VertexBuffer::Create(m_VertexBuffer, m_VertexCount * sizeof(Vertex));
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal"   },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};
		m_VBO->SetLayout(layout);
		m_VAO->AddVertexBuffer(m_VBO);

		Ref<IndexBuffer> IBO = IndexBuffer::Create(m_IndexBuffer, m_IndexCount);
		m_VAO->SetIndexBuffer(IBO);

		delete[] m_VertexBuffer;
		delete[] m_IndexBuffer;
	}

	void Mesh::LoadMeshEntries(const aiScene* meshScene)
	{
		uint32_t meshCount = meshScene->mNumMeshes;
		m_Entries.resize(meshCount);
		for (uint32_t i = 0; i < meshCount; ++i)
		{
			const aiMesh* mesh = meshScene->mMeshes[i];
			m_Entries[i].VertexBufferPtr = m_VertexCount;
			m_Entries[i].IndexBufferPtr = m_IndexCount;
			m_Entries[i].IndexCount = mesh->mNumFaces * 3;
			m_Entries[i].MaterialIndex = mesh->mMaterialIndex;

			m_VertexCount += mesh->mNumVertices;
			m_IndexCount += m_Entries[i].IndexCount;
		}

		// Init material slots with default materials
		m_MaterialSlots.reserve(meshScene->mNumMaterials);
		for (uint32_t i = 0; i < meshScene->mNumMaterials; ++i)
		{
			m_MaterialSlots.emplace_back(MaterialAssetLibrary::GetDefaultMaterialAsset());
		}
	}

	void Mesh::LoadDatas(const aiScene* meshScene)
	{
		for (size_t i = 0; i < m_Entries.size(); ++i)
		{
			const aiMesh* mesh = meshScene->mMeshes[i];
			LoadVertexData(mesh, m_Entries[i].VertexBufferPtr);
			LoadIndexData(mesh, m_Entries[i].IndexBufferPtr);
		}
	}

	void Mesh::LoadVertexData(const aiMesh* mesh, uint32_t baseIndex)
	{
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
		{
			const aiVector3D& position = mesh->mVertices[i];
			const aiVector3D& normal = mesh->mNormals[i];
			const aiVector3D& texCoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D();
			m_VertexBuffer[baseIndex + i].Position = { position.x, position.y, position.z };
			m_VertexBuffer[baseIndex + i].Normal = { normal.x, normal.y, normal.z };
			m_VertexBuffer[baseIndex + i].TexCoord = { normal.x, normal.y };
		}
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

	MeshAsset::MeshAsset(const std::string& path)
		: AssetBase(path)
	{
		Reload();
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

	void MeshAsset::Reload()
	{
		auto meshPath = PathUtils::GetResourcePathFromAssetPath(GetPath());
		ZE_CORE_ASSERT(PathUtils::DoesPathExist(meshPath));
		m_Mesh = Mesh::Create(meshPath);
		Deserialize(); // Do not call it in constructor if it contains shared_from_this()
	}

	void MeshAsset::Serialize(const std::string& path)
	{
		if (path.empty()) return;

		if (path != GetPath())
		{
			SetPath(path);
		}
		ImportableAssetSerializer::Serialize(GetPath(), TypeId(), {}); // TODO: Update component instance here
	}

	void MeshAsset::Deserialize()
	{
		if (GetPath().empty()) return;

		ImportableAssetSerializer::Deserialize(GetPath(), TypeId(), {});  // TODO: Update component instance here
	}

}
