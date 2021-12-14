#include "ZEpch.h"
#include "Engine/Renderer/Shader.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {
	
	Ref<Shader> Shader::Create(const std::string& filePath, bool bIsReload)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLShader>(filePath, bIsReload);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	void Shader::ClearCache(const std::string& filePath)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return;
			case RendererAPI::API::OpenGL:
			{
				std::filesystem::path cacheDirectory = OpenGLShader::GetCacheDirectory();
				std::filesystem::path shaderFilePath = filePath;
				const auto& extensions = OpenGLShader::GetCacheFileExtensions();
				for (const char* extension : extensions)
				{
					const std::string cachePath = (cacheDirectory / (shaderFilePath.filename().string() + extension)).string();
					if (PathUtils::DoesPathExist(cachePath))
					{
						PathUtils::DeletePath(cachePath);
					}
				}
				return;
			}
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return;
		}
	}

	ShaderAsset::ShaderAsset(const std::string& path)
		: AssetBase(path)
	{
		Reload(true);
	}

	Ref<ShaderAsset> ShaderAsset::Create(const std::string& path)
	{
		class ShaderAssetEnableShared : public ShaderAsset
		{
		public:
			explicit ShaderAssetEnableShared(const std::string& path)
				: ShaderAsset(path) {}
		};

		return CreateRef<ShaderAssetEnableShared>(path);
	}

	void ShaderAsset::Reload(bool bIsCreate)
	{
		auto shaderPath = PathUtils::GetResourcePathFromAssetPath(GetPath());
		ZE_CORE_ASSERT(PathUtils::DoesPathExist(shaderPath));

		if (!bIsCreate)
		{
			Shader::ClearCache(shaderPath);
		}
		m_Shader = Shader::Create(shaderPath, !bIsCreate);
		Deserialize();
		if (!bIsCreate)
		{
			m_OnShaderReloadedDel.publish();
		}
	}

	void ShaderAsset::Serialize(const std::string& path)
	{
		if (path.empty()) return;

		if (path != GetPath())
		{
			SetPath(path);
		}
		AssetSerializer::Serialize(GetPath(), TypeId(), {});
	}

	void ShaderAsset::Deserialize()
	{
		if (GetPath().empty()) return;

		AssetSerializer::Deserialize(GetPath(), TypeId(), {});
	}



}