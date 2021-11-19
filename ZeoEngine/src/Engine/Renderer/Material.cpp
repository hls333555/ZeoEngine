#include "ZEpch.h"
#include "Engine/Renderer/Material.h"

#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	void DynamicUniformDataBase::Apply()
	{
		void* valuePtr = GetValuePtr();
		if (!valuePtr) return;

		auto& buffers = OwnerMaterial->GetDynamicUniformBuffers();
		auto& datas = OwnerMaterial->GetDynamicUniformBufferDatas();
		std::copy(reinterpret_cast<char*>(valuePtr), reinterpret_cast<char*>(valuePtr) + Size, datas[Binding] + Offset);
		buffers[Binding]->SetData(datas[Binding]);
	}

	void DynamicUniformBoolData::Draw()
	{
		if (ImGui::Checkbox("##Bool", &Value))
		{
			Apply();
		}
	}

	void DynamicUniformColorData::Draw()
	{
		bool bChanged = ImGui::ColorEdit4("", glm::value_ptr(Value));
		// For dragging
		if (bChanged && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			Apply();
			LastValue = Value;
		}
		// For tabbing (we must force set value back in this case or the buffer will be reset on the next draw)
		if (Input::IsKeyPressed(Key::Tab) && ImGui::GetFocusID() == ImGui::GetItemID())
		{
			if (Value != LastValue)
			{
				Apply();
				LastValue = Value;
			}
		}
		// For multi-component widget, tabbing will switch to the next component, so we must handle deactivation to apply cache first
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (Value != LastValue)
			{
				Apply();
				LastValue = Value;
			}
		}
		if (ImGui::IsItemActivated())
		{
			LastValue = Value;
		}
	}

	void DynamicUniformTexture2DData::Draw()
	{
		// Texture2D asset browser
		auto [bIsBufferChanged, retSpec] = Browser.Draw(Value ? Value->GetPath() : std::string{}, -1.0f, []() {});
		if (bIsBufferChanged)
		{
			Value = retSpec ? Texture2DAssetLibrary::Get().LoadAsset(retSpec->Path) : AssetHandle<Texture2DAsset>{};
			Apply();
		}
	}

	void DynamicUniformTexture2DData::Bind()
	{
		// Bind default texture first
		Texture2D::GetDefaultTexture()->Bind(Binding);
		// Override with our texture if set
		if (Value)
		{
			Value->Bind(Binding);
		}
	}

	void DynamicUniformTexture2DData::Apply()
	{
		Bind();
	}

	Material::Material(const std::string& path)
	{
		m_Shader = m_DefaultShader = ShaderAssetLibrary::GetDefaultShaderAsset();
	}

	Material::~Material()
	{
		for (const auto& [binding, uniformBufferDatas] : m_DynamicUniformBufferDatas)
		{
			delete[] uniformBufferDatas;
		}
	}

	Ref<Shader> Material::GetShader() const
	{
		return m_Shader ? m_Shader->GetShader() : Ref<Shader>{};
	}

	void Material::InitMaterialData()
	{
		InitUniformBuffers();
		ParseReflectionData();
	}

	void Material::Bind()
	{
		if (m_Shader)
		{
			m_Shader->Bind();
			BindUniformDatas();
		}
		else
		{
			m_DefaultShader->Bind();
		}
	}

	void Material::ApplyUniformDatas()
	{
		for (const auto& uniformData : m_DynamicUniforms)
		{
			uniformData->Apply();
		}
	}

	void Material::BindUniformDatas()
	{
		for (const auto& [binding, uniformBuffer] : m_DynamicUniformBuffers)
		{
			uniformBuffer->Bind();
		}
		for (const auto& uniformData : m_DynamicUniforms)
		{
			uniformData->Bind();
		}
	}

	void Material::ParseReflectionData()
	{
		for (const auto& reflectionData : GetShader()->GetShaderReflectionData())
		{
			switch (reflectionData->GetType())
			{
			case ShaderReflectionType::Bool:
				m_DynamicUniforms.emplace_back(CreateScope<DynamicUniformBoolData>(*reflectionData, shared_from_this()));
				break;
			case ShaderReflectionType::Int:
				m_DynamicUniforms.emplace_back(CreateScope<DynamicUniformScalarNData<int32_t>>(*reflectionData, shared_from_this(), ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d"));
				break;
			case ShaderReflectionType::Float:
				m_DynamicUniforms.emplace_back(CreateScope<DynamicUniformScalarNData<float>>(*reflectionData, shared_from_this(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
				break;
			case ShaderReflectionType::Vec2:
				m_DynamicUniforms.emplace_back(CreateScope<DynamicUniformScalarNData<glm::vec2, 2, float>>(*reflectionData, shared_from_this(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
				break;
			case ShaderReflectionType::Vec3:
				m_DynamicUniforms.emplace_back(CreateScope<DynamicUniformScalarNData<glm::vec3, 3, float>>(*reflectionData, shared_from_this(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
				break;
			case ShaderReflectionType::Vec4:
				m_DynamicUniforms.emplace_back(CreateScope<DynamicUniformColorData>(*reflectionData, shared_from_this()));
				break;
			case ShaderReflectionType::Texture2D:
				m_DynamicUniforms.emplace_back(CreateScope<DynamicUniformTexture2DData>(*reflectionData, shared_from_this()));
				break;
			default:
				break;
			}
		}
	}

	void Material::InitUniformBuffers()
	{
		for (const auto& [binding, uniformBlockData] : GetShader()->GetUniformBlockDatas())
		{
			auto uniformBlockSize = uniformBlockData.Size;
			if (m_DynamicUniformBufferDatas.find(binding) != m_DynamicUniformBufferDatas.end())
			{
				delete[] m_DynamicUniformBufferDatas[binding];
			}
			char* bufferData = new char[uniformBlockSize];
			memset(bufferData, 0, uniformBlockSize);
			m_DynamicUniformBufferDatas[binding] = bufferData;

			auto uniformBuffer = UniformBuffer::Create(static_cast<uint32_t>(uniformBlockSize), binding);
			// Upload default 0 values
			uniformBuffer->SetData(bufferData);
			m_DynamicUniformBuffers[binding] = std::move(uniformBuffer);
		}
	}

	MaterialAsset::MaterialAsset(const std::string& path)
		: AssetBase(path)
	{
	}

	Ref<MaterialAsset> MaterialAsset::Create(const std::string& path)
	{
		class MaterialAssetEnableShared : public MaterialAsset
		{
		public:
			explicit MaterialAssetEnableShared(const std::string& path)
				: MaterialAsset(path) {}
		};

		auto asset = CreateRef<MaterialAssetEnableShared>(path);
		asset->Reload();
		return asset;
	}

	void MaterialAsset::Reload()
	{
		m_Material = CreateRef<Material>(GetPath());
		m_Material->InitMaterialData();
		Deserialize();
	}

	void MaterialAsset::Serialize(const std::string& path)
	{
		if (path.empty()) return;

		if (path != GetPath())
		{
			SetPath(path);
		}
		MaterialAssetSerializer::Serialize(GetPath(), TypeId(), MaterialPreviewComponent{ SharedFromBase<MaterialAsset>() }, GetMaterial());
	}

	void MaterialAsset::Deserialize()
	{
		if (GetPath().empty()) return;

		MaterialAssetSerializer::Deserialize(GetPath(), TypeId(), MaterialPreviewComponent{ SharedFromBase<MaterialAsset>() }, GetMaterial());
		// Apply uniform datas after loading
		GetMaterial()->ApplyUniformDatas();
	}

}
