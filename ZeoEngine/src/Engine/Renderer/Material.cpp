#include "ZEpch.h"
#include "Engine/Renderer/Material.h"

#include "Engine/Asset/AssetLibrary.h"

namespace ZeoEngine {

	void DynamicUniformNonMacroFieldBase::ApplyInternal(const void* value, const Ref<Material>& material) const
	{
		if (!value) return;

		auto& buffers = material->GetDynamicUniformBuffers();
		auto& dataContainer = material->GetDynamicUniformBufferDataContainer();
		std::copy(static_cast<const char*>(value), static_cast<const char*>(value) + Size, dataContainer[Binding] + Offset);
		buffers[Binding]->SetData(dataContainer[Binding]);
	}

	void DynamicUniformMacroFieldBase::ApplyInternal(U32 value, const Ref<Material>& material, bool bIsInit) const
	{
		material->SetShaderVariantByMacro(MacroName, value);
		if (!bIsInit)
		{
			// We have to cache current value due to later we will reconstruct these dynamic data
			material->SnapshotDynamicFields();
		}
	}

	void DynamicUniformBoolFieldBase::Draw()
	{
		bool bValue = Value;
		if (ImGui::Checkbox("", &bValue))
		{
			Value = bValue;
			Apply();
		}
	}

	void DynamicUniformBoolField::Apply(bool bIsInit)
	{
		ApplyInternal(GetValueRaw(), OwnerMaterial);
	}

	void DynamicUniformBoolMacroField::Apply(bool bIsInit)
	{
		ApplyInternal(Value, OwnerMaterial, bIsInit);
	}

	void DynamicUniformScalarNMacroField::Draw()
	{
		const I32 min = 0;
		const I32 max = ValueRange - 1;
		void* buffer = bIsEditActive ? &Buffer : &Value;
		// We do not apply during dragging as reloading and reconstructing widgets are not necessary during this operation
		ImGui::DragScalarNEx("", ImGuiDataType_S32, buffer, 1, 0.5f, &min, &max, "%d", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			bIsEditActive = false;
			if (Value != Buffer)
			{
				Apply();
			}
		}
		if (ImGui::IsItemActivated())
		{
			bIsEditActive = true;
			Buffer = Value;
		}
	}

	void DynamicUniformScalarNMacroField::Apply(bool bIsInit)
	{
		if (!bIsInit)
		{
			Value = Buffer;
		}
		ApplyInternal(Value, OwnerMaterial, bIsInit);
	}

	void DynamicUniformColorField::Draw()
	{
		float* buffer = bIsEditActive ? glm::value_ptr(Buffer) : glm::value_ptr(Value);
		bool bChanged = ImGui::ColorEdit4("", buffer);
		if (bChanged && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			Apply();
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			bIsEditActive = false;
			if (Value != Buffer)
			{
				Apply();
			}
		}
		if (ImGui::IsItemActivated())
		{
			bIsEditActive = true;
			Buffer = Value;
		}
	}

	void DynamicUniformColorField::Apply(bool bIsInit)
	{
		if (!bIsInit)
		{
			Value = Buffer;
		}
		ApplyInternal(GetValueRaw(), OwnerMaterial);
	}

	void DynamicUniformTexture2DField::Draw()
	{
		if (Browser.Draw(Value, 0.0f, []() {}))
		{
			Apply();
		}
	}

	void DynamicUniformTexture2DField::Bind() const
	{
		if (const auto texture = AssetLibrary::LoadAsset<Texture2D>(Value))
		{
			texture->SetBindingSlot(Binding);
			texture->Bind();
		}
		else // Bind default texture
		{
			// For diffuse color, we use grey texture, and for others, we use white texture so that most calculations will not be affected
			const auto defaultTexture = Name == "u_DiffuseTexture" ? Texture2D::GetDefaultMaterialTexture() : Texture2D::GetWhiteTexture();
			defaultTexture->SetBindingSlot(Binding);
			defaultTexture->Bind();
		}
	}

	void DynamicUniformTexture2DField::Apply(bool bIsInit)
	{
		Bind();
	}

	Material::Material()
	{
		m_ShaderInstance = CreateRef<ShaderInstance>();
	}

	Material::~Material()
	{
		for (const auto& [binding, uniformBufferData] : m_DynamicUniformBufferDataContainer)
		{
			delete[] uniformBufferData;
		}
		GetShader()->m_OnAssetReloaded.disconnect(this);
	}

	Ref<Material> Material::GetDefaultMaterial()
	{
		return AssetLibrary::LoadAsset<Material>(GetTemplatePath());
	}

	void Material::SetShaderAsset(AssetHandle shaderAsset)
	{
		const auto lastShaderAsset = GetShaderAsset();
		if (shaderAsset != lastShaderAsset)
		{
			m_ShaderInstance->SetShader(shaderAsset);
			OnShaderChanged(shaderAsset, lastShaderAsset);
		}
	}

	void Material::SnapshotDynamicFields()
	{
		MaterialSerializer ms;
		ms.Serialize(m_DynamicFieldCache, SharedFromThis());
	}

	void Material::RestoreDynamicFields()
	{
		MaterialSerializer ms;
		ms.Deserialize(m_DynamicFieldCache, SharedFromThis());
		m_DynamicFieldCache.reset();
	}

	void Material::ReloadShaderDataAndApplyDynamicFields()
	{
		if (!GetShader()->GatherReflectionData(GetShaderVariant())) return;

		InitMaterialData();
		RestoreDynamicFields();
		ApplyDynamicFields();
	}

	void Material::ReloadShaderDataAndDeserialize()
	{
		const auto shader = GetShader();
		if (!shader->IsVariantValid(GetShaderVariant()))
		{
			// Set default shader variant if current one is not valid due to
			// 1. material asset does not have a ShaderVariant data to deserialize
			// 2. shader content has changed so that current variant no longer exists
			SetShaderVariant(shader->GetDefaultVariant()->ID);
		}

		if (!shader->GatherReflectionData(GetShaderVariant())) return;

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

	void Material::ApplyDynamicFields() const
	{
		for (const auto& field : m_DynamicFields)
		{
			field->Apply(true);
		}
	}

	void Material::InitMaterialData()
	{
		m_Techniques.clear();
		m_DynamicFields.clear();
		m_DynamicFieldCategoryLocations.clear();
		m_DynamicBindableFields.clear();
		m_DynamicUniformBuffers.clear();
		m_DynamicUniformBufferDataContainer.clear();

		InitUniformBuffers();
		ConstructDynamicFields();
		InitRenderTechniques();

		m_OnMaterialInitializedDel.publish(SharedFromThis());
	}

	void Material::InitUniformBuffers()
	{
		for (const auto& [binding, uniformBlockSize] : GetShader()->GetUniformBlockSizes())
		{
			if (m_DynamicUniformBufferDataContainer.find(binding) != m_DynamicUniformBufferDataContainer.end())
			{
				delete[] m_DynamicUniformBufferDataContainer[binding];
			}
			char* bufferData = new char[uniformBlockSize];
			memset(bufferData, 0, uniformBlockSize);
			m_DynamicUniformBufferDataContainer[binding] = bufferData;

			auto uniformBuffer = UniformBuffer::Create(static_cast<U32>(uniformBlockSize), binding);
			// Upload default 0 values
			uniformBuffer->SetData(bufferData);
			m_DynamicUniformBuffers[binding] = std::move(uniformBuffer);
		}
	}

	void Material::ConstructDynamicFields()
	{
		const auto shader = GetShader();
		for (const auto& reflectionData : shader->GetShaderReflectionMacroFields())
		{
			switch (reflectionData->GetType())
			{
				case ShaderReflectionFieldType::Bool:
					m_DynamicFields.emplace_back(CreateRef<DynamicUniformBoolMacroField>(*reflectionData, SharedFromThis()));
					break;
				case ShaderReflectionFieldType::Int:
					{
						const auto& intData = dynamic_cast<ShaderReflectionIntMacroField&>(*reflectionData);
						m_DynamicFields.emplace_back(CreateRef<DynamicUniformScalarNMacroField>(intData, SharedFromThis(), intData.ValueRange));
					}
					break;
				default:
					break;
			}
		}
		for (const auto& reflectionData : shader->GetShaderReflectionFields())
		{
			switch (reflectionData->GetType())
			{
				case ShaderReflectionFieldType::Bool:
					m_DynamicFields.emplace_back(CreateRef<DynamicUniformBoolField>(*reflectionData, SharedFromThis()));
					break;
				case ShaderReflectionFieldType::Int:
					m_DynamicFields.emplace_back(CreateRef<DynamicUniformScalarNField<I32>>(*reflectionData, SharedFromThis()));
					break;
				case ShaderReflectionFieldType::Float:
					m_DynamicFields.emplace_back(CreateRef<DynamicUniformScalarNField<float>>(*reflectionData, SharedFromThis()));
					break;
				case ShaderReflectionFieldType::Vec2:
					m_DynamicFields.emplace_back(CreateRef<DynamicUniformScalarNField<Vec2, 2, float>>(*reflectionData, SharedFromThis()));
					break;
				case ShaderReflectionFieldType::Vec3:
					m_DynamicFields.emplace_back(CreateRef<DynamicUniformScalarNField<Vec3, 3, float>>(*reflectionData, SharedFromThis()));
					break;
				case ShaderReflectionFieldType::Vec4:
					m_DynamicFields.emplace_back(CreateRef<DynamicUniformColorField>(*reflectionData, SharedFromThis()));
					break;
				case ShaderReflectionFieldType::Texture2D:
					m_DynamicBindableFields.emplace_back(CreateRef<DynamicUniformTexture2DField>(*reflectionData, SharedFromThis()));
					break;
				default:
					break;
			}
		}

		// Sort data by category
		std::sort(m_DynamicFields.begin(), m_DynamicFields.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs->Category < rhs->Category;
		});

		// Record category location in m_DynamicFields
		std::string lastCategory;
		for (SizeT i = 0; i < m_DynamicFields.size(); ++i)
		{
			const std::string& category = m_DynamicFields[i]->Category;
			if (category != lastCategory)
			{
				m_DynamicFieldCategoryLocations.emplace_back(i);
			}
			lastCategory = category;
		}
		m_DynamicFieldCategoryLocations.emplace_back(m_DynamicFields.size());
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
				for (const auto& uniformBindableField : m_DynamicBindableFields)
				{
					step.AddBindable(uniformBindableField);
				}
				step.AddBindable(m_ShaderInstance);
				shade.AddStep(std::move(step));
			}
			m_Techniques.emplace_back(std::move(shade));
		}
	}

	void Material::OnShaderChanged(AssetHandle shaderAsset, AssetHandle lastShaderAsset)
	{
		if (const auto lastShader = AssetLibrary::LoadAsset<Shader>(lastShaderAsset))
		{
			lastShader->m_OnAssetReloaded.disconnect(this);
		}
		const auto shader = AssetLibrary::LoadAsset<Shader>(shaderAsset);
		ZE_CORE_ASSERT(shaderAsset); // Setting invalid shader is not allowed
		shader->m_OnAssetReloaded.connect<&Material::ReloadShaderDataAndDeserialize>(this);
		ReloadShaderDataAndDeserialize();
	}

}
