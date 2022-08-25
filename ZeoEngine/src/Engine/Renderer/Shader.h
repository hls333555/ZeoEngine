#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Bindable.h"
#include "Engine/Asset/Asset.h"

namespace ZeoEngine {

	enum class ShaderReflectionType
	{
		None, Bool, Float, Int, Vec2, Vec3, Vec4, Texture2D,
	};

	struct ShaderReflectionDataBase
	{
		std::string BufferName;
		std::string Name;

		ShaderReflectionDataBase(std::string bufferName, std::string name)
			: BufferName(std::move(bufferName)), Name(std::move(name)) {}
		virtual ~ShaderReflectionDataBase() = default;

		virtual ShaderReflectionType GetType() const = 0 { return ShaderReflectionType::None; }
	};

	struct ShaderReflectionNonMacroDataBase : public ShaderReflectionDataBase
	{
		U32 Binding = 0;
		U32 Offset = 0;
		SizeT Size = 0;

		ShaderReflectionNonMacroDataBase(std::string bufferName, std::string name, U32 binding, U32 offset = 0, SizeT size = 0)
			: ShaderReflectionDataBase(std::move(bufferName), std::move(name)), Binding(binding), Offset(offset), Size(size) {}
	};

	struct ShaderReflectionMacroDataBase : public ShaderReflectionDataBase
	{
		std::string MacroName;

		ShaderReflectionMacroDataBase(std::string bufferName, std::string name, std::string macroName)
			: ShaderReflectionDataBase(std::move(bufferName), std::move(name))
			, MacroName(std::move(macroName)) {}
	};

	struct ShaderReflectionBoolData : public ShaderReflectionNonMacroDataBase
	{
		using ShaderReflectionNonMacroDataBase::ShaderReflectionNonMacroDataBase;

		bool Value = false;

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Bool; }
	};

	struct ShaderReflectionBoolMacroData : public ShaderReflectionMacroDataBase
	{
		using ShaderReflectionMacroDataBase::ShaderReflectionMacroDataBase;

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Bool; }
	};

	struct ShaderReflectionIntData : public ShaderReflectionNonMacroDataBase
	{
		using ShaderReflectionNonMacroDataBase::ShaderReflectionNonMacroDataBase;

		I32 Value = 0;

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Int; }
	};

	struct ShaderReflectionIntMacroData : public ShaderReflectionMacroDataBase
	{
		U32 ValueRange;

		ShaderReflectionIntMacroData(std::string bufferName, std::string name, std::string macroName, U32 valueRange)
			: ShaderReflectionMacroDataBase(std::move(bufferName), std::move(name), std::move(macroName))
			, ValueRange(valueRange) {}

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Int; }
	};

	struct ShaderReflectionFloatData : public ShaderReflectionNonMacroDataBase
	{
		using ShaderReflectionNonMacroDataBase::ShaderReflectionNonMacroDataBase;

		float Value = 0.0f;

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Float; }
	};

	struct ShaderReflectionVec2Data : public ShaderReflectionNonMacroDataBase
	{
		using ShaderReflectionNonMacroDataBase::ShaderReflectionNonMacroDataBase;

		Vec2 Value{ 0.0f };

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Vec2; }
	};

	struct ShaderReflectionVec3Data : public ShaderReflectionNonMacroDataBase
	{
		using ShaderReflectionNonMacroDataBase::ShaderReflectionNonMacroDataBase;

		Vec3 Value{ 0.0f };

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Vec3; }
	};

	struct ShaderReflectionVec4Data : public ShaderReflectionNonMacroDataBase
	{
		using ShaderReflectionNonMacroDataBase::ShaderReflectionNonMacroDataBase;

		Vec4 Value{ 0.0f };

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Vec4; }
	};

	struct ShaderReflectionTexture2DData : public ShaderReflectionNonMacroDataBase
	{
		using ShaderReflectionNonMacroDataBase::ShaderReflectionNonMacroDataBase;

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Texture2D; }
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

		virtual const std::vector<Scope<ShaderReflectionNonMacroDataBase>>& GetShaderReflectionData() const = 0;
		virtual const std::vector<Scope<ShaderReflectionMacroDataBase>>& GetShaderReflectionMacroData() const = 0;
		virtual const std::unordered_map <U32, SizeT>& GetUniformBlockSizes() const = 0;

		virtual void ClearCache() const = 0;

	};

	class ShaderInstance : public Bindable
	{
	public:
		ShaderInstance() = default;
		ShaderInstance(Ref<Shader> shader)
			: m_Shader(std::move(shader))
			, m_ShaderVariantID(m_Shader->GetDefaultVariant()->ID) {}

		virtual void Bind() const override { m_Shader->SetActiveRendererIDByID(m_ShaderVariantID); m_Shader->Bind(); }

		const Ref<Shader>& GetShader() const { return m_Shader; }
		void SetShader(const Ref<Shader>& shader) { m_Shader = shader; }
		U32 GetShaderVariant() const { return m_ShaderVariantID; }
		void SetShaderVariant(U32 ID) { m_ShaderVariantID = ID; }
		void SetShaderVariantByMacro(const std::string& name, U32 value);

	private:
		Ref<Shader> m_Shader;
		U32 m_ShaderVariantID = 0;
	};

}
