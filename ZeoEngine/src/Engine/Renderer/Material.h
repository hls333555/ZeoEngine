#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <yaml-cpp/yaml.h>

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/ImGui/AssetBrowser.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/RenderTechnique.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	class Material;

	struct DynamicUniformFieldBase
	{
		std::string Name;
		std::string Category;

		Ref<Material> OwnerMaterial;

		DynamicUniformFieldBase(std::string name, std::string category, const Ref<Material>& material)
			: Name(std::move(name))
			, Category(std::move(category))
			, OwnerMaterial(material) {}
		virtual ~DynamicUniformFieldBase() = default;

		virtual ShaderReflectionFieldType GetFieldType() const { return ShaderReflectionFieldType::None; }
		virtual void Draw() = 0;
		virtual void* GetValueRaw() = 0;
		/** Called during material initialization or when value is changed in the editor. */
		virtual void Apply(bool bIsInit = false) = 0;
	};

	struct DynamicUniformNonMacroFieldBase
	{
		U32 Binding = 0;
		U32 Offset = 0;
		SizeT Size = 0;

		DynamicUniformNonMacroFieldBase(U32 binding, U32 offset, SizeT size)
			: Binding(binding), Offset(offset), Size(size) {}

		void ApplyInternal(const void* value, const Ref<Material>& material) const;
	};

	struct DynamicUniformMacroFieldBase
	{
		std::string MacroName;

		explicit DynamicUniformMacroFieldBase(std::string macroName)
			: MacroName(std::move(macroName)) {}

		void ApplyInternal(U32 value, const Ref<Material>& material, bool bIsInit) const;
	};

	template<typename Type>
	struct DynamicUniformFieldBufferBase : public DynamicUniformFieldBase
	{
		using DynamicUniformFieldBase::DynamicUniformFieldBase;

		Type Value;
		Type Buffer;
		bool bIsEditActive = false;
	};

	struct DynamicUniformBoolFieldBase : public DynamicUniformFieldBase
	{
		using DynamicUniformFieldBase::DynamicUniformFieldBase;

		U32 Value = 0; // SpirV reflects bool as UInt

		virtual ShaderReflectionFieldType GetFieldType() const override { return ShaderReflectionFieldType::Bool; }
		virtual void Draw() override;
		virtual void* GetValueRaw() override { return &Value; }
	};

	struct DynamicUniformBoolField : public DynamicUniformBoolFieldBase, public DynamicUniformNonMacroFieldBase
	{
		DynamicUniformBoolField(const ShaderReflectionNonMacroFieldBase& reflectionData, const Ref<Material>& material)
			: DynamicUniformBoolFieldBase(reflectionData.Name, reflectionData.BufferName, material), DynamicUniformNonMacroFieldBase(reflectionData.Binding, reflectionData.Offset, reflectionData.Size) {}

		virtual void Apply(bool bIsInit = false) override;
	};

	struct DynamicUniformBoolMacroField : public DynamicUniformBoolFieldBase, public DynamicUniformMacroFieldBase
	{
		DynamicUniformBoolMacroField(const ShaderReflectionMacroFieldBase& reflectionData, const Ref<Material>& material)
			: DynamicUniformBoolFieldBase(reflectionData.Name, reflectionData.BufferName, material), DynamicUniformMacroFieldBase(reflectionData.MacroName) {}

		virtual void Apply(bool bIsInit = false) override;
	};

	template<typename Type, U32 N = 1, typename BaseType = Type>
	struct DynamicUniformScalarNFieldBase : public DynamicUniformFieldBufferBase<Type>
	{
		DynamicUniformScalarNFieldBase(std::string name, std::string category, const Ref<Material>& material)
			: DynamicUniformFieldBufferBase(std::move(name), std::move(category), material) {}

		virtual ShaderReflectionFieldType GetFieldType() const override
		{
			if constexpr (std::is_same_v<Type, I32>)
			{
				return ShaderReflectionFieldType::Int;
			}
			else if constexpr (std::is_same_v<Type, float>)
			{
				return ShaderReflectionFieldType::Float;
			}
			else if constexpr (std::is_same_v<Type, Vec2>)
			{
				return ShaderReflectionFieldType::Vec2;
			}
			else if constexpr (std::is_same_v<Type, Vec3>)
			{
				return ShaderReflectionFieldType::Vec3;
			}

			ZE_CORE_ASSERT(false);
			return ShaderReflectionFieldType::None;
		}

		virtual void Draw() override
		{
			const auto min = EngineUtils::GetDefaultMin<BaseType>();
			const auto max = EngineUtils::GetDefaultMax<BaseType>();
			void* buffer = nullptr;
			if constexpr (N == 1)
			{
				buffer =  bIsEditActive ? &Buffer : &Value;
			}
			else
			{
				buffer = bIsEditActive ? glm::value_ptr(Buffer) : glm::value_ptr(Value);
			}
			bool bChanged = ImGui::DragScalarNEx("", ShaderReflectionFieldTypeToImGuiDataType(), buffer, N, 0.5f, &min, &max, GetOutputFormatByShaderReflectionFieldType(), ImGuiSliderFlags_AlwaysClamp);
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

		virtual void* GetValueRaw() override
		{
			if constexpr (N == 1)
			{
				return &Value;
			}
			else
			{
				return glm::value_ptr(Value);
			}
		}

	private:
		ImGuiDataType ShaderReflectionFieldTypeToImGuiDataType() const
		{
			if constexpr (std::is_same_v<Type, I32>)
			{
				return ImGuiDataType_S32;
			}
			else
			{
				return ImGuiDataType_Float;
			}
		}

		const char* GetOutputFormatByShaderReflectionFieldType() const
		{
			if constexpr (std::is_same_v<Type, I32>)
			{
				return "%d";
			}
			else
			{
				return "%.3f";
			}
		}
	};

	template<typename Type, U32 N = 1, typename BaseType = Type>
	struct DynamicUniformScalarNField : public DynamicUniformScalarNFieldBase<Type, N, BaseType>, public DynamicUniformNonMacroFieldBase
	{
		DynamicUniformScalarNField(const ShaderReflectionNonMacroFieldBase& reflectionData, const Ref<Material>& material)
			: DynamicUniformScalarNFieldBase(reflectionData.Name, reflectionData.BufferName, material), DynamicUniformNonMacroFieldBase(reflectionData.Binding, reflectionData.Offset, reflectionData.Size)
		{
			static_assert(N == 1 || N == 2 || N == 3, "N can only be 1, 2 or 3!");
		}

		virtual void Apply(bool bIsInit = false) override
		{
			if (!bIsInit)
			{
				Value = Buffer;
			}
			ApplyInternal(GetValueRaw(), OwnerMaterial);
		}
	};

	struct DynamicUniformScalarNMacroField : public DynamicUniformScalarNFieldBase<I32, 1, I32>, public DynamicUniformMacroFieldBase
	{
		I32 ValueRange = 0;

		DynamicUniformScalarNMacroField(const ShaderReflectionMacroFieldBase& reflectionData, const Ref<Material>& material, I32 valueRange)
			: DynamicUniformScalarNFieldBase(reflectionData.Name, reflectionData.BufferName, material), DynamicUniformMacroFieldBase(reflectionData.MacroName)
			, ValueRange(valueRange) {}

		virtual void Draw() override;
		virtual void Apply(bool bIsInit = false) override;
	};

	struct DynamicUniformColorField : public DynamicUniformFieldBufferBase<Vec4>, public DynamicUniformNonMacroFieldBase
	{
		DynamicUniformColorField(const ShaderReflectionNonMacroFieldBase& reflectionData, const Ref<Material>& material)
			: DynamicUniformFieldBufferBase(reflectionData.Name, reflectionData.BufferName, material), DynamicUniformNonMacroFieldBase(reflectionData.Binding, reflectionData.Offset, reflectionData.Size) {}

		virtual ShaderReflectionFieldType GetFieldType() const override { return ShaderReflectionFieldType::Vec4; }
		virtual void Draw() override;
		virtual void* GetValueRaw() override { return glm::value_ptr(Value); }
		virtual void Apply(bool bIsInit = false) override;
	};

	struct DynamicUniformTexture2DField : public DynamicUniformFieldBase, public DynamicUniformNonMacroFieldBase, public Bindable
	{
		AssetHandle Value;
		AssetBrowser Browser{ Texture2D::TypeID() };

		DynamicUniformTexture2DField(const ShaderReflectionNonMacroFieldBase& reflectionData, const Ref<Material>& material)
			: DynamicUniformFieldBase(reflectionData.Name, reflectionData.BufferName, material), DynamicUniformNonMacroFieldBase(reflectionData.Binding, reflectionData.Offset, reflectionData.Size) {}

		virtual ShaderReflectionFieldType GetFieldType() const override { return ShaderReflectionFieldType::Texture2D; }
		virtual void Draw() override;
		virtual void* GetValueRaw() override { return &Value; }
		virtual void Bind() const override; // Called from RenderStep::Bind()
		virtual void Apply(bool bIsInit = false) override;
	};

	class Material : public AssetBase<Material>
	{
		friend struct MaterialPreviewComponent;

	public:
		Material();
		Material(Material&&) = default; // Defined as RenderTechnique cannot be copied
		virtual ~Material();

		static constexpr const char* GetTemplatePath() { return "Engine/materials/NewMaterial.zasset"; }
		static Ref<Material> GetDefaultMaterial();

		AssetHandle GetShaderAsset() const { return m_ShaderInstance->GetShaderAsset(); }
		Ref<Shader> GetShader() const { return m_ShaderInstance->GetShader(); }
		void SetShaderAsset(AssetHandle shaderAsset);
		void OnShaderChanged(AssetHandle shaderAsset, AssetHandle lastShaderAsset);
		U32 GetShaderVariant() const { return m_ShaderInstance->GetShaderVariant(); }
		void SetShaderVariant(U32 ID) const { m_ShaderInstance->SetShaderVariant(ID); }
		void SetShaderVariantByMacro(const std::string& name, U32 value) const { m_ShaderInstance->SetShaderVariantByMacro(name, value); }

		const auto& GetDynamicFields() const { return m_DynamicFields; }
		const auto& GetDynamicFieldCategoryLocations() const { return m_DynamicFieldCategoryLocations; }
		const auto& GetDynamicBindableFields() const { return m_DynamicBindableFields; }
		auto& GetDynamicUniformBuffers() { return m_DynamicUniformBuffers; }
		auto& GetDynamicUniformBufferDataContainer() { return m_DynamicUniformBufferDataContainer; }
		const auto& GetRenderTechniques() const { return m_Techniques; }

		bool IsDynamicFieldCacheAvailable() const { return m_DynamicFieldCache.size() != 0; }
		void SnapshotDynamicFields();
		void RestoreDynamicFields();

		void ReloadShaderDataAndDeserialize();
		void ReloadShaderDataAndApplyDynamicFields();
		void ApplyDynamicFields() const;

	private:
		void InitMaterialData();
		void InitUniformBuffers();
		void ConstructDynamicFields();
		void InitRenderTechniques();

	public:
		entt::sink<entt::sigh<void(const Ref<Material>&)>> m_OnMaterialInitialized{ m_OnMaterialInitializedDel };

	private:
		Ref<ShaderInstance> m_ShaderInstance;

		/** Macro fields first, uniform fields second and ordered by category */
		std::vector<Ref<DynamicUniformFieldBase>> m_DynamicFields;
		/** Stores a series of location of categories in dynamic fields, used for drawing widgets in MaterialInspector */
		std::vector<SizeT> m_DynamicFieldCategoryLocations;
		std::vector<Ref<DynamicUniformTexture2DField>> m_DynamicBindableFields;

		/** Map from uniform block binding to uniform buffer */
		std::unordered_map<U32, Ref<UniformBuffer>> m_DynamicUniformBuffers;
		/** Map from uniform block binding to uniform buffer data */
		std::unordered_map<U32, char*> m_DynamicUniformBufferDataContainer;

		/** Dynamic field values stored in YAML format */
		YAML::Node m_DynamicFieldCache;

		std::vector<RenderTechnique> m_Techniques;

		entt::sigh<void(const Ref<Material>&)> m_OnMaterialInitializedDel;
	};

}
