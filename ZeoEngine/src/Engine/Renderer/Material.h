#pragma once

#include <glm/gtc/type_ptr.hpp>

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
		uint32_t Binding = 0;
		uint32_t Offset = 0;
		size_t Size = 0;

		AssetHandle<Material> OwnerMaterial;

		DynamicUniformDataBase(const ShaderReflectionDataBase& reflectionData, const AssetHandle<Material>& material)
			: Name(reflectionData.Name), Binding(reflectionData.Binding), Offset(reflectionData.Offset), Size(reflectionData.Size)
			, OwnerMaterial(material) {}

		virtual ShaderReflectionType GetDataType() const = 0 { return ShaderReflectionType::None; }
		virtual void Draw() = 0;
		virtual void* GetValuePtr() = 0;
		/** Called in material initialization or when value changes. */
		virtual void Apply();
	};

	struct DynamicUniformBoolData : public DynamicUniformDataBase
	{
		using DynamicUniformDataBase::DynamicUniformDataBase;

		bool Value = false;

		virtual ShaderReflectionType GetDataType() const override { return ShaderReflectionType::Bool; }
		virtual void Draw() override;
		virtual void* GetValuePtr() override { return &Value; }
	};

	template<typename T, uint32_t N = 1, typename CT = T>
	struct DynamicUniformScalarNData : public DynamicUniformDataBase
	{
		T Value = T();
		T LastValue = T();

		ImGuiDataType ScalarType;
		CT DefaultMin, DefaultMax;
		const char* Format = nullptr;

		DynamicUniformScalarNData(const ShaderReflectionDataBase& reflectionData, const AssetHandle<Material>& material, ImGuiDataType scalarType, CT defaultMin, CT defaultMax, const char* format)
			: DynamicUniformDataBase(reflectionData, material)
			, ScalarType(scalarType)
			, DefaultMin(defaultMin), DefaultMax(defaultMax)
			, Format(format)
		{
			static_assert(N == 1 || N == 2 || N == 3, "N can only be 1, 2 or 3!");
		}

		virtual ShaderReflectionType GetDataType() const override
		{
			if constexpr (std::is_same<T, int32_t>::value)
			{
				return ShaderReflectionType::Int;
			}
			else if constexpr (std::is_same<T, float>::value)
			{
				return ShaderReflectionType::Float;
			}
			else if constexpr (std::is_same<T, glm::vec2>::value)
			{
				return ShaderReflectionType::Vec2;
			}
			else if constexpr (std::is_same<T, glm::vec3>::value)
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

	struct DynamicUniformColorData : public DynamicUniformDataBase
	{
		using DynamicUniformDataBase::DynamicUniformDataBase;

		glm::vec4 Value{ 0.0f };
		glm::vec4 LastValue{ 0.0f };

		virtual ShaderReflectionType GetDataType() const override { return ShaderReflectionType::Vec4; }
		virtual void Draw() override;
		virtual void* GetValuePtr() override { return glm::value_ptr(Value); }
	};

	struct DynamicUniformTexture2DData : public DynamicUniformDataBase, public Bindable
	{
		AssetBrowser Browser{ Texture2D::TypeId() };
		AssetHandle<Texture2D> Value;

		DynamicUniformTexture2DData(const ShaderReflectionDataBase& reflectionData, const AssetHandle<Material>& material)
			: DynamicUniformDataBase(reflectionData, material) {}

		virtual ShaderReflectionType GetDataType() const override { return ShaderReflectionType::Texture2D; }
		virtual void Draw() override;
		virtual void* GetValuePtr() override { return &Value; }
		virtual void Bind() const override;
		virtual void Apply() override;
	};

	class Material : public AssetBase<Material>
	{
	public:
		explicit Material(const std::string& path);
		virtual ~Material();

		static Ref<Material> Create(const std::string& path, bool bIsReload);

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		const AssetHandle<Shader>& GetShader() const { return m_Shader; }
		void SetShader(const AssetHandle<Shader>& shader) { m_Shader = shader; }

		const auto& GetDynamicUniforms() const { return m_DynamicUniforms; }
		const auto& GetDynamicBindableUniforms() const { return m_DynamicBindableUniforms; }
		auto& GetDynamicUniformBuffers() { return m_DynamicUniformBuffers; }
		auto& GetDynamicUniformBufferDatas() { return m_DynamicUniformBufferDatas; }
		const auto& GetRenderTechniques() const { return m_Techniques; }

		void InitMaterialData();
		void ApplyUniformDatas() const;

	private:
		void ParseReflectionData();
		void InitUniformBuffers();
		void Reload();

	public:
		entt::sink<entt::sigh<void(const AssetHandle<Material>&)>> m_OnMaterialInitialized{ m_OnMaterialInitializedDel };

	private:
		AssetHandle<Shader> m_DefaultShader, m_Shader;
		std::vector<Ref<DynamicUniformDataBase>> m_DynamicUniforms;
		std::vector<Ref<DynamicUniformTexture2DData>> m_DynamicBindableUniforms;
		/** Map from uniform block binding to uniform buffers */
		std::unordered_map<uint32_t, Ref<UniformBuffer>> m_DynamicUniformBuffers;
		/** Map from uniform block binding to uniform buffer datas */
		std::unordered_map<uint32_t, char*> m_DynamicUniformBufferDatas;

		std::vector<RenderTechnique> m_Techniques;

		entt::sigh<void(const AssetHandle<Material>&)> m_OnMaterialInitializedDel;
	};

	REGISTER_ASSET(Material,
	Ref<Material> operator()(const std::string& path, bool bIsReload) const
	{
		return Material::Create(path, bIsReload);
	},
	static AssetHandle<Material> GetDefaultMaterial()
	{
		return Get().LoadAsset("assets/editor/materials/DefaultMaterial.zasset");
	})

}
