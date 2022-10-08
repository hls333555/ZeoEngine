#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Bindable.h"
#include "Engine/Asset/Asset.h"

namespace ZeoEngine {

	enum class ShaderReflectionFieldType
	{
		None, Bool, Int, Float, Vec2, Vec3, Vec4, Texture2D,
	};

	struct ShaderReflectionFieldBase
	{
		std::string BufferName;
		std::string Name;

		ShaderReflectionFieldBase(std::string bufferName, std::string name)
			: BufferName(std::move(bufferName)), Name(std::move(name)) {}
		virtual ~ShaderReflectionFieldBase() = default;

		virtual ShaderReflectionFieldType GetType() const = 0 { return ShaderReflectionFieldType::None; }
	};

	struct ShaderReflectionNonMacroFieldBase : public ShaderReflectionFieldBase
	{
		U32 Binding = 0;
		U32 Offset = 0;
		SizeT Size = 0;

		ShaderReflectionNonMacroFieldBase(std::string bufferName, std::string name, U32 binding, U32 offset = 0, SizeT size = 0)
			: ShaderReflectionFieldBase(std::move(bufferName), std::move(name)), Binding(binding), Offset(offset), Size(size) {}
	};

	struct ShaderReflectionMacroFieldBase : public ShaderReflectionFieldBase
	{
		std::string MacroName;

		ShaderReflectionMacroFieldBase(std::string bufferName, std::string name, std::string macroName)
			: ShaderReflectionFieldBase(std::move(bufferName), std::move(name))
			, MacroName(std::move(macroName)) {}
	};

	struct ShaderReflectionBoolField : public ShaderReflectionNonMacroFieldBase
	{
		using ShaderReflectionNonMacroFieldBase::ShaderReflectionNonMacroFieldBase;

		bool Value = false;

		virtual ShaderReflectionFieldType GetType() const override { return ShaderReflectionFieldType::Bool; }
	};

	struct ShaderReflectionBoolMacroField : public ShaderReflectionMacroFieldBase
	{
		using ShaderReflectionMacroFieldBase::ShaderReflectionMacroFieldBase;

		virtual ShaderReflectionFieldType GetType() const override { return ShaderReflectionFieldType::Bool; }
	};

	struct ShaderReflectionIntField : public ShaderReflectionNonMacroFieldBase
	{
		using ShaderReflectionNonMacroFieldBase::ShaderReflectionNonMacroFieldBase;

		I32 Value = 0;

		virtual ShaderReflectionFieldType GetType() const override { return ShaderReflectionFieldType::Int; }
	};

	struct ShaderReflectionIntMacroField : public ShaderReflectionMacroFieldBase
	{
		U32 ValueRange;

		ShaderReflectionIntMacroField(std::string bufferName, std::string name, std::string macroName, U32 valueRange)
			: ShaderReflectionMacroFieldBase(std::move(bufferName), std::move(name), std::move(macroName))
			, ValueRange(valueRange) {}

		virtual ShaderReflectionFieldType GetType() const override { return ShaderReflectionFieldType::Int; }
	};

	struct ShaderReflectionFloatField : public ShaderReflectionNonMacroFieldBase
	{
		using ShaderReflectionNonMacroFieldBase::ShaderReflectionNonMacroFieldBase;

		float Value = 0.0f;

		virtual ShaderReflectionFieldType GetType() const override { return ShaderReflectionFieldType::Float; }
	};

	struct ShaderReflectionVec2Field : public ShaderReflectionNonMacroFieldBase
	{
		using ShaderReflectionNonMacroFieldBase::ShaderReflectionNonMacroFieldBase;

		Vec2 Value{ 0.0f };

		virtual ShaderReflectionFieldType GetType() const override { return ShaderReflectionFieldType::Vec2; }
	};

	struct ShaderReflectionVec3Field : public ShaderReflectionNonMacroFieldBase
	{
		using ShaderReflectionNonMacroFieldBase::ShaderReflectionNonMacroFieldBase;

		Vec3 Value{ 0.0f };

		virtual ShaderReflectionFieldType GetType() const override { return ShaderReflectionFieldType::Vec3; }
	};

	struct ShaderReflectionVec4Field : public ShaderReflectionNonMacroFieldBase
	{
		using ShaderReflectionNonMacroFieldBase::ShaderReflectionNonMacroFieldBase;

		Vec4 Value{ 0.0f };

		virtual ShaderReflectionFieldType GetType() const override { return ShaderReflectionFieldType::Vec4; }
	};

	struct ShaderReflectionTexture2DField : public ShaderReflectionNonMacroFieldBase
	{
		using ShaderReflectionNonMacroFieldBase::ShaderReflectionNonMacroFieldBase;

		virtual ShaderReflectionFieldType GetType() const override { return ShaderReflectionFieldType::Texture2D; }
	};

	struct ShaderVariantData
	{
		std::map<std::string, std::string> Macros; // Map can guarantee the order

		ShaderVariantData(std::map<std::string, std::string> macros)
			: Macros(std::move(macros)) {}

		void SetMacro(std::string name, std::string value)
		{
			Macros[std::move(name)] = std::move(value);
		}

		U32 GetID() const
		{
			std::string res;
			for (const auto& [name, value] : Macros)
			{
				res += name + value;
			}
			return entt::hashed_string(res.c_str());
		}

		operator bool() const
		{
			return !Macros.empty();
		}
	};

	struct ShaderVariant
	{
		Scope<ShaderVariantData> Data;
		U32 ID = 0;
		U32 RendererID = 0;
		std::unordered_map<U32, std::vector<U32>> VulkanSPIRV;

		ShaderVariant() = default;
		ShaderVariant(Scope<ShaderVariantData> data)
			: Data(std::move(data))
			, ID(Data->GetID()) {}
	};

	class Shader : public Bindable, public AssetBase<Shader>
	{
	public:
		virtual ~Shader() = default;

		static Ref<Shader> Create(std::string resourcePath);
		static Ref<Shader> Create(const std::string& vertexSrc, const std::string& fragmentSrc);

		Ref<class ShaderInstance> CreateInstance();

		static constexpr const char* GetTemplatePath() { return "Engine/shaders/NewShader.glsl.zasset"; }
		static constexpr const char* GetResourceTemplatePath() { return "assets/shaders/NewShader.glsl"; }

		void Reload();

		virtual void SetActiveRendererIDByID(U32 variantID) = 0;
		virtual Ref<ShaderVariant> GetVariantByID(U32 ID) const = 0;
		virtual bool IsVariantValid(U32 ID) const = 0;
		virtual Ref<ShaderVariant> GetDefaultVariant() const = 0;

		virtual bool ParseAndCompile() = 0;
		virtual bool Compile(const Ref<ShaderVariant>& variant) = 0;
		virtual bool GatherReflectionData(U32 variantID) = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, U32 count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const Vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const Vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const Vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const Mat4& value) = 0;

		virtual const std::vector<Scope<ShaderReflectionNonMacroFieldBase>>& GetShaderReflectionFields() const = 0;
		virtual const std::vector<Scope<ShaderReflectionMacroFieldBase>>& GetShaderReflectionMacroFields() const = 0;
		virtual const std::unordered_map <U32, SizeT>& GetUniformBlockSizes() const = 0;

		virtual void ClearCache() const = 0;

	};

	class ShaderInstance : public Bindable
	{
		friend struct MaterialPreviewComponent;

	public:
		ShaderInstance() = default;
		ShaderInstance(const Ref<Shader>& shader)
			: m_ShaderAsset(shader->GetHandle())
			, m_ShaderVariantID(shader->GetDefaultVariant()->ID) {}

		virtual void Bind() const override;

		AssetHandle GetShaderAsset() const { return m_ShaderAsset; }
		Ref<Shader> GetShader() const;
		void SetShader(AssetHandle shader) { m_ShaderAsset = shader; }
		U32 GetShaderVariant() const { return m_ShaderVariantID; }
		void SetShaderVariant(U32 ID) { m_ShaderVariantID = ID; }
		void SetShaderVariantByMacro(const std::string& name, U32 value);

	private:
		AssetHandle m_ShaderAsset;
		U32 m_ShaderVariantID = 0;
	};

}
