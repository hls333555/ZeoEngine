#pragma once

#include <glm/glm.hpp>

#include "Engine/Core/Asset.h"
#include "Engine/Core/AssetLibrary.h"

namespace ZeoEngine {

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
