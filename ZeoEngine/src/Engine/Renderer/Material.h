#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <yaml-cpp/yaml.h>

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/ImGui/AssetBrowser.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Core/Input.h"
#include "Engine/Renderer/RenderTechnique.h"

namespace ZeoEngine {

	class Material;
	
	struct DynamicUniformDataBase
	{
		std::string Name;
		std::string Category;

		Ref<Material> OwnerMaterial;

		DynamicUniformDataBase(std::string name, std::string category, const Ref<Material>& material)
			: Name(std::move(name))
			, Category(std::move(category))
			, OwnerMaterial(material) {}
		virtual ~DynamicUniformDataBase() = default;

		virtual ShaderReflectionType GetDataType() const { return ShaderReflectionType::None; }
		virtual void Draw() = 0;
		virtual void* GetValuePtr() = 0;
		/** Called during material initialization or when value changes. */
		virtual void Apply(bool bIsInit = false) = 0;
	};

	struct DynamicUniformNonMacroDataBase
	{
		U32 Binding = 0;
		U32 Offset = 0;
		SizeT Size = 0;

		DynamicUniformNonMacroDataBase(U32 binding, U32 offset, SizeT size)
			: Binding(binding), Offset(offset), Size(size) {}

		void ApplyInternal(void* valuePtr, const Ref<Material>& material) const;
	};

	struct DynamicUniformMacroDataBase
	{
		std::string MacroName;

		explicit DynamicUniformMacroDataBase(std::string macroName)
			: MacroName(std::move(macroName)) {}

		void ApplyInternal(bool bIsInit, const Ref<Material>& material, U32 value) const;
	};

	struct DynamicUniformBoolDataBase : public DynamicUniformDataBase
	{
		using DynamicUniformDataBase::DynamicUniformDataBase;

		bool bValue = false;
		U32 Value = 0; // SpirV reflects bool as UInt

		virtual ShaderReflectionType GetDataType() const override { return ShaderReflectionType::Bool; }
		virtual void Draw() override;
		virtual void* GetValuePtr() override { return &Value; }
	};

	struct DynamicUniformBoolData : public DynamicUniformBoolDataBase, public DynamicUniformNonMacroDataBase
	{
		DynamicUniformBoolData(const ShaderReflectionNonMacroDataBase& reflectionData, const Ref<Material>& material)
			: DynamicUniformBoolDataBase(reflectionData.Name, reflectionData.BufferName, material), DynamicUniformNonMacroDataBase(reflectionData.Binding, reflectionData.Offset, reflectionData.Size) {}

		virtual void Apply(bool bIsInit = false) override;
	};

	struct DynamicUniformBoolMacroData : public DynamicUniformBoolDataBase, public DynamicUniformMacroDataBase
	{
		DynamicUniformBoolMacroData(const ShaderReflectionMacroDataBase& reflectionData, const Ref<Material>& material)
			: DynamicUniformBoolDataBase(reflectionData.Name, reflectionData.BufferName, material), DynamicUniformMacroDataBase(reflectionData.MacroName) {}

		virtual void Apply(bool bIsInit = false) override;
	};

	template<typename T, U32 N = 1, typename CT = T>
	struct DynamicUniformScalarNDataBase : public DynamicUniformDataBase
	{
		T Value = T();
		T LastValue = T();

		ImGuiDataType ScalarType;
		CT DefaultMin, DefaultMax;
		const char* Format = nullptr;

		DynamicUniformScalarNDataBase(std::string name, std::string category, const Ref<Material>& material, ImGuiDataType scalarType, CT defaultMin, CT defaultMax, const char* format)
			: DynamicUniformDataBase(std::move(name), std::move(category), material)
			, ScalarType(scalarType)
			, DefaultMin(defaultMin), DefaultMax(defaultMax)
			, Format(format)
		{
		}

		virtual ShaderReflectionType GetDataType() const override
		{
			if constexpr (std::is_same<T, I32>::value)
			{
				return ShaderReflectionType::Int;
			}
			else if constexpr (std::is_same<T, float>::value)
			{
				return ShaderReflectionType::Float;
			}
			else if constexpr (std::is_same<T, Vec2>::value)
			{
				return ShaderReflectionType::Vec2;
			}
			else if constexpr (std::is_same<T, Vec3>::value)
			{
				return ShaderReflectionType::Vec3;
			}

			return ShaderReflectionType::None;
		}

		virtual void Draw() override
		{
			bool bChanged = ImGui::DragScalarNEx("", ScalarType, GetValuePtr(), N, 0.5f, &DefaultMin, &DefaultMax, Format, ImGuiSliderFlags_AlwaysClamp);
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

		virtual void* GetValuePtr() override
		{
			void* valuePtr = nullptr;
			if constexpr (N == 1)
			{
				valuePtr = &Value;
			}
			else
			{
				valuePtr = glm::value_ptr(Value);
			}
			return valuePtr;
		}
	};

	template<typename T, U32 N = 1, typename CT = T>
	struct DynamicUniformScalarNData : public DynamicUniformScalarNDataBase<T, N, CT>, public DynamicUniformNonMacroDataBase
	{
		DynamicUniformScalarNData(const ShaderReflectionNonMacroDataBase& reflectionData, const Ref<Material>& material, ImGuiDataType scalarType, CT defaultMin, CT defaultMax, const char* format)
			: DynamicUniformScalarNDataBase(reflectionData.Name, reflectionData.BufferName, material, scalarType, defaultMin, defaultMax, format), DynamicUniformNonMacroDataBase(reflectionData.Binding, reflectionData.Offset, reflectionData.Size)
		{
			static_assert(N == 1 || N == 2 || N == 3, "N can only be 1, 2 or 3!");
		}

		virtual void Apply(bool bIsInit = false) override
		{
			ApplyInternal(GetValuePtr(), OwnerMaterial);
		}
	};

	struct DynamicUniformScalarNMacroData : public DynamicUniformScalarNDataBase<I32, 1, I32>, public DynamicUniformMacroDataBase
	{
		DynamicUniformScalarNMacroData(const ShaderReflectionMacroDataBase& reflectionData, const Ref<Material>& material, ImGuiDataType scalarType, I32 defaultMin, I32 defaultMax, const char* format)
			: DynamicUniformScalarNDataBase(reflectionData.Name, reflectionData.BufferName, material, scalarType, defaultMin, defaultMax, format), DynamicUniformMacroDataBase(reflectionData.MacroName)
		{
		}

		virtual void Draw() override
		{
			// We do not apply during dragging as reloading and reconstructing widgets are not necessary during this operation
			ImGui::DragScalarNEx("", ScalarType, GetValuePtr(), 1, 0.5f, &DefaultMin, &DefaultMax, Format, ImGuiSliderFlags_AlwaysClamp);
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

		virtual void Apply(bool bIsInit = false) override
		{
			ApplyInternal(bIsInit, OwnerMaterial, Value);
		}
	};

	struct DynamicUniformColorData : public DynamicUniformDataBase, public DynamicUniformNonMacroDataBase
	{
		DynamicUniformColorData(const ShaderReflectionNonMacroDataBase& reflectionData, const Ref<Material>& material)
			: DynamicUniformDataBase(reflectionData.Name, reflectionData.BufferName, material), DynamicUniformNonMacroDataBase(reflectionData.Binding, reflectionData.Offset, reflectionData.Size) {}

		Vec4 Value{ 0.0f };
		Vec4 LastValue{ 0.0f };

		virtual ShaderReflectionType GetDataType() const override { return ShaderReflectionType::Vec4; }
		virtual void Draw() override;
		virtual void* GetValuePtr() override { return glm::value_ptr(Value); }
		virtual void Apply(bool bIsInit = false) override;
	};

	struct DynamicUniformTexture2DData : public DynamicUniformDataBase, public DynamicUniformNonMacroDataBase, public Bindable
	{
		AssetBrowser Browser{ Texture2D::TypeID() };
		Ref<Texture2D> Value;

		DynamicUniformTexture2DData(const ShaderReflectionNonMacroDataBase& reflectionData, const Ref<Material>& material)
			: DynamicUniformDataBase(reflectionData.Name, reflectionData.BufferName, material), DynamicUniformNonMacroDataBase(reflectionData.Binding, reflectionData.Offset, reflectionData.Size) {}

		virtual ShaderReflectionType GetDataType() const override { return ShaderReflectionType::Texture2D; }
		virtual void Draw() override;
		virtual void* GetValuePtr() override { return &Value; }
		virtual void Bind() const override; // Called from RenderStep::Bind()
		virtual void Apply(bool bIsInit = false) override;
	};

	class Material : public AssetBase<Material>
	{
	public:
		Material();
		Material(Material&&) = default; // Defined as RenderTechnique cannot be copied
		virtual ~Material();

		static constexpr const char* GetTemplatePath() { return "assets/editor/materials/NewMaterial.zasset"; }
		static Ref<Material> GetDefaultMaterial();

		const Ref<Shader>& GetShader() const { return m_ShaderInstance->GetShader(); }
		void SetShader(const Ref<Shader>& shader);
		U32 GetShaderVariant() const { return m_ShaderInstance->GetShaderVariant(); }
		void SetShaderVariant(U32 ID) { m_ShaderInstance->SetShaderVariant(ID); }
		void SetShaderVariantByMacro(const std::string& name, U32 value) { m_ShaderInstance->SetShaderVariantByMacro(name, value); }

		const auto& GetDynamicData() const { return m_DynamicData; }
		const auto& GetDynamicDataCategoryLocations() const { return m_DynamicDataCategoryLocations; }
		const auto& GetDynamicBindableData() const { return m_DynamicBindableData; }
		auto& GetDynamicUniformBuffers() { return m_DynamicUniformBuffers; }
		auto& GetDynamicUniformBufferDatas() { return m_DynamicUniformBufferDatas; }
		const auto& GetRenderTechniques() const { return m_Techniques; }

		bool IsSnapshotDynamicDataAvailable() const { return m_SnapshotDynamicData.size() != 0; }
		void SnapshotDynamicData();
		void RestoreSnapshotDynamicData();

		void ReloadShaderDataAndDeserialize();
		void ReloadShaderDataAndApplyDynamicData();
		void ApplyDynamicData() const;

	private:
		void InitMaterialData();
		void InitUniformBuffers();
		void ConstructDynamicData();
		void InitRenderTechniques();

	public:
		entt::sink<entt::sigh<void(const Ref<Material>&)>> m_OnMaterialInitialized{ m_OnMaterialInitializedDel };

	private:
		Ref<ShaderInstance> m_ShaderInstance;

		/** Macro data first, uniform data second and ordered by category */
		std::vector<Ref<DynamicUniformDataBase>> m_DynamicData;
		/** Stores a series of location of categories in m_DynamicData, used for drawing widgets in MaterialInspector */
		std::vector<SizeT> m_DynamicDataCategoryLocations;
		std::vector<Ref<DynamicUniformTexture2DData>> m_DynamicBindableData;
		/** Map from uniform block binding to uniform buffers */
		std::unordered_map<U32, Ref<UniformBuffer>> m_DynamicUniformBuffers;
		/** Map from uniform block binding to uniform buffer datas */
		std::unordered_map<U32, char*> m_DynamicUniformBufferDatas;
		YAML::Node m_SnapshotDynamicData;

		std::vector<RenderTechnique> m_Techniques;

		entt::sigh<void(const Ref<Material>&)> m_OnMaterialInitializedDel;
	};

}
