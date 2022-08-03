#include "ZEpch.h"
#include "Engine/Renderer/Material.h"

#include "Engine/Asset/AssetLibrary.h"

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
		bValue = Value;
		if (ImGui::Checkbox("##Bool", &bValue))
		{
			Value = bValue;
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

	void DynamicUniformTexture2DData::Apply()
	{
		Bind();
	}

	Material::Material()
	{
		m_Shader = Shader::GetDefaultShader();
		m_Shader->m_OnAssetReloaded.connect<&Material::ReloadShaderData>(this);
	}

	Material::~Material()
	{
		for (const auto& [binding, uniformBufferDatas] : m_DynamicUniformBufferDatas)
		{
			delete[] uniformBufferDatas;
		}
		m_Shader->m_OnAssetReloaded.disconnect(this);
	}

	Ref<Material> Material::GetDefaultMaterial()
	{
		return AssetLibrary::LoadAsset<Material>(GetTemplatePath());
	}

	void Material::ReloadShaderData()
	{
		auto* serializer = AssetManager::Get().GetAssetSerializerByAssetType(TypeID());
		const auto* materialSerializer = dynamic_cast<MaterialAssetSerializer*>(serializer);
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(GetHandle());
		materialSerializer->ReloadShaderData(metadata, SharedFromThis());
	}

	void Material::InitMaterialData()
	{
		m_Techniques.clear();
		m_DynamicUniforms.clear();
		m_DynamicBindableUniforms.clear();
		m_DynamicUniformBuffers.clear();
		m_DynamicUniformBufferDatas.clear();
		InitUniformBuffers();
		ParseReflectionData();

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
				for (const auto& uniformBindableData : m_DynamicBindableUniforms)
				{
					step.AddBindable(uniformBindableData);
				}
				step.AddBindable(m_Shader);
				shade.AddStep(std::move(step));
			}
			m_Techniques.emplace_back(std::move(shade));
		}

		m_OnMaterialInitializedDel.publish(SharedFromThis());
	}

	void Material::ApplyUniformDatas() const
	{
		for (const auto& uniformData : m_DynamicUniforms)
		{
			uniformData->Apply();
		}
	}

	void Material::ParseReflectionData()
	{
		for (const auto& reflectionData : m_Shader->GetShaderReflectionData())
		{
			switch (reflectionData->GetType())
			{
			case ShaderReflectionType::Bool:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformBoolData>(*reflectionData, SharedFromThis()));
				break;
			case ShaderReflectionType::Int:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformScalarNData<I32>>(*reflectionData, SharedFromThis(), ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d"));
				break;
			case ShaderReflectionType::Float:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformScalarNData<float>>(*reflectionData, SharedFromThis(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
				break;
			case ShaderReflectionType::Vec2:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformScalarNData<Vec2, 2, float>>(*reflectionData, SharedFromThis(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
				break;
			case ShaderReflectionType::Vec3:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformScalarNData<Vec3, 3, float>>(*reflectionData, SharedFromThis(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
				break;
			case ShaderReflectionType::Vec4:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformColorData>(*reflectionData, SharedFromThis()));
				break;
			case ShaderReflectionType::Texture2D:
				m_DynamicBindableUniforms.emplace_back(CreateRef<DynamicUniformTexture2DData>(*reflectionData, SharedFromThis()));
				break;
			default:
				break;
			}
		}
	}

	void Material::InitUniformBuffers()
	{
		for (const auto& [binding, uniformBlockData] : m_Shader->GetUniformBlockDatas())
		{
			auto uniformBlockSize = uniformBlockData.Size;
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

}
