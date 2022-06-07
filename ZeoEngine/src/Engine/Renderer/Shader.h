#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Bindable.h"
#include "Engine/Core/Asset.h"
#include "Engine/Core/AssetLibrary.h"

namespace ZeoEngine {

	enum class ShaderReflectionType
	{
		None, Bool, Float, Int, Vec2, Vec3, Vec4, Texture2D,
	};

	struct ShaderReflectionDataBase
	{
		std::string Name;
		U32 Binding = 0;
		U32 Offset = 0;
		SizeT Size = 0;

		ShaderReflectionDataBase(std::string name, U32 binding, U32 offset = 0, SizeT size = 0)
			: Name(std::move(name)), Binding(binding), Offset(offset), Size(size) {}
		virtual ~ShaderReflectionDataBase() = default;

		virtual ShaderReflectionType GetType() const = 0 { return ShaderReflectionType::None; }
	};

	struct ShaderReflectionBoolData : public ShaderReflectionDataBase
	{
		using ShaderReflectionDataBase::ShaderReflectionDataBase;

		bool Value = false;

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Bool; }
	};

	struct ShaderReflectionIntData : public ShaderReflectionDataBase
	{
		using ShaderReflectionDataBase::ShaderReflectionDataBase;

		I32 Value = 0;

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Int; }
	};

	struct ShaderReflectionFloatData : public ShaderReflectionDataBase
	{
		using ShaderReflectionDataBase::ShaderReflectionDataBase;

		float Value = 0.0f;

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Float; }
	};

	struct ShaderReflectionVec2Data : public ShaderReflectionDataBase
	{
		using ShaderReflectionDataBase::ShaderReflectionDataBase;

		Vec2 Value{ 0.0f };

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Vec2; }
	};

	struct ShaderReflectionVec3Data : public ShaderReflectionDataBase
	{
		using ShaderReflectionDataBase::ShaderReflectionDataBase;

		Vec3 Value{ 0.0f };

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Vec3; }
	};

	struct ShaderReflectionVec4Data : public ShaderReflectionDataBase
	{
		using ShaderReflectionDataBase::ShaderReflectionDataBase;

		Vec4 Value{ 0.0f };

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Vec4; }
	};

	struct ShaderReflectionTexture2DData : public ShaderReflectionDataBase
	{
		using ShaderReflectionDataBase::ShaderReflectionDataBase;

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Texture2D; }
	};

	struct UniformBlockData
	{
		/** Name of the buffer */
		std::string Name;
		/** Size of the buffer */
		SizeT Size;
		/** Map to begin and end index of Material::m_DynamicUniforms */
		SizeT BeginIndex, EndIndex;
	};

	class Shader : public Bindable, public AssetBase<Shader>
	{
	public:
		explicit Shader(std::string ID)
			: AssetBase(std::move(ID)) {}
		virtual ~Shader() = default;

		static Ref<Shader> Create(const std::string& path);
		static Ref<Shader> Create(std::string ID, const std::string& vertexSrc, const std::string& fragmentSrc);

		static constexpr const char* GetTemplatePath() { return "assets/editor/shaders/NewShader.glsl.zasset"; }
		static constexpr const char* GetResourceTemplatePath() { return "assets/editor/shaders/NewShader.glsl"; }

		virtual void Reload() override;
		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		virtual void ParseAndCompile() = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, U32 count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const Vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const Vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const Vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const Mat4& value) = 0;

		virtual const std::vector<Scope<ShaderReflectionDataBase>>& GetShaderReflectionData() const = 0;
		virtual SizeT GetResourceCount() const = 0;
		virtual const std::unordered_map <U32, UniformBlockData>& GetUniformBlockDatas() const = 0;

		static void ClearCache(const std::string& path);

	};

	REGISTER_ASSET(Shader,
	Ref<Shader> operator()(const std::string& path) const
	{
		return Shader::Create(path);
	},
	// TODO:
	static AssetHandle<Shader> GetDefaultShader()
	{
		return ShaderLibrary::Get().LoadAsset("assets/editor/shaders/Default.glsl.zasset");
	})

}
