#include "ZEpch.h"
#include "Engine/Renderer/Material.h"

#include "Engine/Asset/AssetLibrary.h"

namespace ZeoEngine {

	void DynamicUniformNonMacroDataBase::ApplyInternal(void* valuePtr, const Ref<Material>& material) const
	{
		if (!valuePtr) return;

		auto& buffers = material->GetDynamicUniformBuffers();
		auto& datas = material->GetDynamicUniformBufferDatas();
		std::copy(reinterpret_cast<char*>(valuePtr), reinterpret_cast<char*>(valuePtr) + Size, datas[Binding] + Offset);
		buffers[Binding]->SetData(datas[Binding]);
	}

	void DynamicUniformMacroDataBase::ApplyInternal(bool bIsInit, const Ref<Material>& material, U32 value) const
	{
		material->SetShaderVariantByMacro(MacroName, value);
		if (!bIsInit)
		{
			// We have to cache current values due to later we will reconstruct these dynamic data
			material->SnapshotDynamicData();
		}
	}

	void DynamicUniformBoolDataBase::Draw()
	{
		bValue = Value;
		if (ImGui::Checkbox("##Bool", &bValue))
		{
			Value = bValue;
			Apply();
		}
	}

	void DynamicUniformBoolData::Apply(bool bIsInit)
	{
		ApplyInternal(GetValuePtr(), OwnerMaterial);
	}

	void DynamicUniformBoolMacroData::Apply(bool bIsInit)
	{
		ApplyInternal(bIsInit, OwnerMaterial, Value);
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

	void DynamicUniformColorData::Apply(bool bIsInit)
	{
		ApplyInternal(GetValuePtr(), OwnerMaterial);
	}

	void DynamicUniformTexture2DData::Draw()
	{
		// Texture2D asset browser
		auto [bIsBufferChanged, metadata] = Browser.Draw(Value ? Value->GetHandle() : 0, -1.0f, []() {});
		if (bIsBufferChanged)
		{
			Value = metadata ? AssetLibrary::LoadAsset<Texture2D>(metadata->Path) : nullptr;
			Apply();
		}
	}

	void DynamicUniformTexture2DData::Bind() const
	{
		if (Value)
		{
			Value->SetBindingSlot(Binding);
			Value->Bind();
		}
		else // Bind default texture
		{
			// For diffuse color, we use grey texture, and for others, we use white texture so that most calculations will not be affected
			const auto defaultTexture = Name == "u_DiffuseTexture" ? Texture2D::GetDefaultMaterialTexture() : Texture2D::GetWhiteTexture();
			defaultTexture->SetBindingSlot(Binding);
			defaultTexture->Bind();
		}
	}

	void DynamicUniformTexture2DData::Apply(bool bIsInit)
	{
		Bind();
	}

	Material::Material()
	{
		m_ShaderInstance = CreateRef<ShaderInstance>();
	}

	Material::~Material()
	{
		for (const auto& [binding, uniformBufferDatas] : m_DynamicUniformBufferDatas)
		{
			delete[] uniformBufferDatas;
		}
		GetShader()->m_OnAssetReloaded.disconnect(this);
	}

	Ref<Material> Material::GetDefaultMaterial()
	{
		return AssetLibrary::LoadAsset<Material>(GetTemplatePath());
	}

	void Material::SetShader(const Ref<Shader>& shader)
	{
		if (const auto& lastShader = GetShader())
		{
			lastShader->m_OnAssetReloaded.disconnect(this);
		}
		m_ShaderInstance->SetShader(shader);
		GetShader()->m_OnAssetReloaded.connect<&Material::ReloadShaderDataAndDeserialize>(this);
		ReloadShaderDataAndDeserialize();
	}

	void Material::SnapshotDynamicData()
	{
		MaterialSerializer ms;
		ms.Serialize(m_SnapshotDynamicData, SharedFromThis());
	}

	void Material::RestoreSnapshotDynamicData()
	{
		MaterialSerializer ms;
		ms.Deserialize(m_SnapshotDynamicData, SharedFromThis());
		m_SnapshotDynamicData.reset();
	}

	void Material::ReloadShaderDataAndApplyDynamicData()
	{
		if (!GetShader()->GatherReflectionData(GetShaderVariant())) return;

		InitMaterialData();
		RestoreSnapshotDynamicData();
		ApplyDynamicData();
	}

	void Material::ReloadShaderDataAndDeserialize()
	{
		if (!GetShader()->IsVariantValid(GetShaderVariant()))
		{
			// Set default shader variant if current one is not valid due to
			// 1. material asset does not have a ShaderVariant data to deserialize
			// 2. shader content has changed so that current variant no longer exists
			SetShaderVariant(GetShader()->GetDefaultVariant()->ID);
		}

		if (!GetShader()->GatherReflectionData(GetShaderVariant())) return;

		InitMaterialData();
		auto* serializer = AssetManager::Get().GetAssetSerializerByAssetType(TypeID());
		const auto* materialSerializer = dynamic_cast<MaterialAssetSerializer*>(serializer);
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(GetHandle());
		const U32 lastShaderVariant = GetShaderVariant();
		// Deserialize data and possibly apply macro value
		materialSerializer->DeserializeShaderData(metadata, SharedFromThis());
		// If any macro value after deserializing and applying is different from current variant's, we regather reflection data based on the new variant and deserialize again
		// so that certain saved values can be deserialized properly (imagine "UseNormalMapping" will toggle existence of "u_NormalTexture")
		if (GetShaderVariant() != lastShaderVariant)
		{
			ReloadShaderDataAndDeserialize();
		}
	}

	void Material::ApplyDynamicData() const
	{
		for (const auto& data : m_DynamicData)
		{
			data->Apply(true);
		}
	}

	void Material::InitMaterialData()
	{
		m_Techniques.clear();
		m_DynamicData.clear();
		m_DynamicDataCategoryLocations.clear();
		m_DynamicBindableData.clear();
		m_DynamicUniformBuffers.clear();
		m_DynamicUniformBufferDatas.clear();

		InitUniformBuffers();
		ConstructDynamicData();
		InitRenderTechniques();

		m_OnMaterialInitializedDel.publish(SharedFromThis());
	}

	void Material::InitUniformBuffers()
	{
		for (const auto& [binding, uniformBlockSize] : GetShader()->GetUniformBlockSizes())
		{
			if (m_DynamicUniformBufferDatas.find(binding) != m_DynamicUniformBufferDatas.end())
			{
				delete[] m_DynamicUniformBufferDatas[binding];
			}
			char* bufferData = new char[uniformBlockSize];
			memset(bufferData, 0, uniformBlockSize);
			m_DynamicUniformBufferDatas[binding] = bufferData;

			auto uniformBuffer = UniformBuffer::Create(static_cast<U32>(uniformBlockSize), binding);
			// Upload default 0 values
			uniformBuffer->SetData(bufferData);
			m_DynamicUniformBuffers[binding] = std::move(uniformBuffer);
		}
	}

	void Material::ConstructDynamicData()
	{
		for (const auto& reflectionData : GetShader()->GetShaderReflectionMacroData())
		{
			switch (reflectionData->GetType())
			{
				case ShaderReflectionType::Bool:
					m_DynamicData.emplace_back(CreateRef<DynamicUniformBoolMacroData>(*reflectionData, SharedFromThis()));
					break;
				case ShaderReflectionType::Int:
					{
						const auto& intData = dynamic_cast<ShaderReflectionIntMacroData&>(*reflectionData);
						m_DynamicData.emplace_back(CreateRef<DynamicUniformScalarNMacroData>(intData, SharedFromThis(), ImGuiDataType_S32, 0, intData.ValueRange - 1, "%d"));
					}
					break;
				default:
					break;
			}
		}
		for (const auto& reflectionData : GetShader()->GetShaderReflectionData())
		{
			switch (reflectionData->GetType())
			{
				case ShaderReflectionType::Bool:
					m_DynamicData.emplace_back(CreateRef<DynamicUniformBoolData>(*reflectionData, SharedFromThis()));
					break;
				case ShaderReflectionType::Int:
					m_DynamicData.emplace_back(CreateRef<DynamicUniformScalarNData<I32>>(*reflectionData, SharedFromThis(), ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d"));
					break;
				case ShaderReflectionType::Float:
					m_DynamicData.emplace_back(CreateRef<DynamicUniformScalarNData<float>>(*reflectionData, SharedFromThis(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
					break;
				case ShaderReflectionType::Vec2:
					m_DynamicData.emplace_back(CreateRef<DynamicUniformScalarNData<Vec2, 2, float>>(*reflectionData, SharedFromThis(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
					break;
				case ShaderReflectionType::Vec3:
					m_DynamicData.emplace_back(CreateRef<DynamicUniformScalarNData<Vec3, 3, float>>(*reflectionData, SharedFromThis(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
					break;
				case ShaderReflectionType::Vec4:
					m_DynamicData.emplace_back(CreateRef<DynamicUniformColorData>(*reflectionData, SharedFromThis()));
					break;
				case ShaderReflectionType::Texture2D:
					m_DynamicBindableData.emplace_back(CreateRef<DynamicUniformTexture2DData>(*reflectionData, SharedFromThis()));
					break;
				default:
					break;
			}
		}

		// Sort data by category
		std::sort(m_DynamicData.begin(), m_DynamicData.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs->Category < rhs->Category;
		});

		// Record category location in m_DynamicData
		std::string lastCategory;
		for (SizeT i = 0; i < m_DynamicData.size(); ++i)
		{
			const std::string& category = m_DynamicData[i]->Category;
			if (category != lastCategory)
			{
				m_DynamicDataCategoryLocations.emplace_back(i);
			}
			lastCategory = category;
		}
		m_DynamicDataCategoryLocations.emplace_back(m_DynamicData.size());
	}

	void Material::InitRenderTechniques()
	{
		{
			RenderTechnique shadow("Shadow");
			{
				RenderStep step1("ShadowMapping");
				shadow.AddStep(std::move(step1));
				RenderStep step2("ScreenSpaceShadow");
				shadow.AddStep(std::move(step2));
			}
			m_Techniques.emplace_back(std::move(shadow));
		}
		{
			RenderTechnique shade("Shade");
			{
				RenderStep step("Opaque");
				for (const auto& [binding, uniformBuffer] : m_DynamicUniformBuffers)
				{
					step.AddBindable(uniformBuffer);
				}
				for (const auto& uniformBindableData : m_DynamicBindableData)
				{
					step.AddBindable(uniformBindableData);
				}
				step.AddBindable(m_ShaderInstance);
				shade.AddStep(std::move(step));
			}
			m_Techniques.emplace_back(std::move(shade));
		}
	}

}
