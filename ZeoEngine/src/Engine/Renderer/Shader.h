#pragma once

#include <glm/glm.hpp>

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
		uint32_t Binding = 0;
		uint32_t Offset = 0;
		size_t Size = 0;

		ShaderReflectionDataBase(const std::string& name, uint32_t binding, uint32_t offset = 0, size_t size = 0)
			: Name(name), Binding(binding), Offset(offset), Size(size) {}

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

		int32_t Value = 0;

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

		glm::vec2 Value{ 0.0f };

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Vec2; }
	};

	struct ShaderReflectionVec3Data : public ShaderReflectionDataBase
	{
		using ShaderReflectionDataBase::ShaderReflectionDataBase;

		glm::vec3 Value{ 0.0f };

		virtual ShaderReflectionType GetType() const override { return ShaderReflectionType::Vec3; }
	};

	struct ShaderReflectionVec4Data : public ShaderReflectionDataBase
	{
		using ShaderReflectionDataBase::ShaderReflectionDataBase;

		glm::vec4 Value{ 0.0f };

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
		size_t Size;
		/** Map to begin and end index of Material::m_DynamicUniforms */
		size_t BeginIndex, EndIndex;
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual const std::string& GetName() const = 0;
		virtual const std::vector<Scope<ShaderReflectionDataBase>>& GetShaderReflectionData() const = 0;
		virtual size_t GetResourceCount() const = 0;
		virtual const std::unordered_map <uint32_t, UniformBlockData>& GetUniformBlockDatas() const = 0;

		static Ref<Shader> Create(const std::string& filePath);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);

	};

	class ShaderAsset : public AssetBase<ShaderAsset>
	{
	private:
		explicit ShaderAsset(const std::string& path);

	public:
		static Ref<ShaderAsset> Create(const std::string& path);

		const Ref<Shader>& GetShader() const { return m_Shader; }

		void Bind() const { m_Shader->Bind(); }

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		virtual void Reload() override;

	private:
		Ref<Shader> m_Shader;
	};

	struct ShaderAssetLoader final : AssetLoader<ShaderAssetLoader, ShaderAsset>
	{
		AssetHandle<ShaderAsset> load(const std::string& path) const
		{
			return ShaderAsset::Create(path);
		}
	};

	class ShaderAssetLibrary : public AssetLibrary<ShaderAssetLibrary, ShaderAsset, ShaderAssetLoader>
	{
	public:
		static AssetHandle<ShaderAsset> GetDefaultShaderAsset()
		{
			return ShaderAssetLibrary::Get().LoadAsset("assets/editor/shaders/Default.glsl.zasset");
		}
	};

}
