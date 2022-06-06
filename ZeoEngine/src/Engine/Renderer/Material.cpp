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
		auto [bIsBufferChanged, retSpec] = Browser.Draw(Value ? Value->GetID() : std::string{}, -1.0f, []() {});
		if (bIsBufferChanged)
		{
			Value = retSpec ? Texture2DLibrary::Get().LoadAsset(retSpec->Path) : AssetHandle<Texture2D>{};
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
		else
		{
			// Bind default texture
			const auto defaultTexture = Texture2DLibrary::GetDefaultMaterialTexture();
			defaultTexture->SetBindingSlot(Binding);
			defaultTexture->Bind();
		}
	}

	void DynamicUniformTexture2DData::Apply()
	{
		Bind();
	}

	Material::Material(const std::string& path)
		: AssetBase(path)
	{
		m_Shader = ShaderLibrary::GetDefaultShader();
	}

	Material::~Material()
	{
		for (const auto& [binding, uniformBufferDatas] : m_DynamicUniformBufferDatas)
		{
			delete[] uniformBufferDatas;
		}
		// TODO: Verfy this
		m_Shader->m_OnShaderReloaded.disconnect(this);
	}

	Ref<Material> Material::Create(const std::string& path, bool bIsReload)
	{
		auto material = CreateRef<Material>(path);
		material->Reload();
		if (!bIsReload)
		{
			material->GetShader()->m_OnShaderReloaded.connect<&Material::Reload>(material);
		}
		return material;
	}

	void Material::Reload()
	{
		InitMaterialData();
		m_OnMaterialInitializedDel.publish(GetAssetHandle());
		Deserialize();
	}

	void Material::Serialize(const std::string& path)
	{
		std::string assetPath = PathUtils::GetNormalizedAssetPath(path);
		if (!PathUtils::DoesPathExist(assetPath)) return;

		SetID(std::move(assetPath));
		MaterialAssetSerializer::Serialize(GetID(), TypeId(), MaterialPreviewComponent{ GetAssetHandle() }, GetAssetHandle());
	}

	void Material::Deserialize()
	{
		if (!PathUtils::DoesPathExist(GetID())) return;

		MaterialAssetSerializer::Deserialize(GetID(), TypeId(), MaterialPreviewComponent{ GetAssetHandle() }, GetAssetHandle());
		// Apply uniform datas after loading
		ApplyUniformDatas();
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
				step.AddBindable(m_Shader.to_ref());
				shade.AddStep(std::move(step));
			}
			m_Techniques.emplace_back(std::move(shade));
		}
		{
			RenderTechnique shadow("Shadow");
			{
				RenderStep step("ShadowMapping");
				shadow.AddStep(std::move(step));
			}
			m_Techniques.emplace_back(std::move(shadow));
		}
		{
			RenderTechnique shadow("Shadow");
			{
				RenderStep step("ScreenSpaceShadow");
				shadow.AddStep(std::move(step));
			}
			m_Techniques.emplace_back(std::move(shadow));
		}
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
		for (const auto& reflectionData : GetShader()->GetShaderReflectionData())
		{
			switch (reflectionData->GetType())
			{
			case ShaderReflectionType::Bool:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformBoolData>(*reflectionData, GetAssetHandle()));
				break;
			case ShaderReflectionType::Int:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformScalarNData<I32>>(*reflectionData, GetAssetHandle(), ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d"));
				break;
			case ShaderReflectionType::Float:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformScalarNData<float>>(*reflectionData, GetAssetHandle(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
				break;
			case ShaderReflectionType::Vec2:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformScalarNData<Vec2, 2, float>>(*reflectionData, GetAssetHandle(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
				break;
			case ShaderReflectionType::Vec3:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformScalarNData<Vec3, 3, float>>(*reflectionData, GetAssetHandle(), ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f"));
				break;
			case ShaderReflectionType::Vec4:
				m_DynamicUniforms.emplace_back(CreateRef<DynamicUniformColorData>(*reflectionData, GetAssetHandle()));
				break;
			case ShaderReflectionType::Texture2D:
				m_DynamicBindableUniforms.emplace_back(CreateRef<DynamicUniformTexture2DData>(*reflectionData, GetAssetHandle()));
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

			auto uniformBuffer = UniformBuffer::Create(static_cast<U32>(uniformBlockSize), binding);
			// Upload default 0 values
			uniformBuffer->SetData(bufferData);
			m_DynamicUniformBuffers[binding] = std::move(uniformBuffer);
		}
	}

}
