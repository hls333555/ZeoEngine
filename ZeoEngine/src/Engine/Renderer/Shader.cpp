#include "ZEpch.h"
#include "Engine/Renderer/Shader.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {
	
	Ref<Shader> Shader::Create(const std::string& path, bool bIsReload)
	{
		std::string resourcePath = PathUtils::GetResourcePathFromPath(path);
		if (!PathUtils::DoesPathExist(resourcePath)) return {};

		if (bIsReload)
		{
			ClearCache(resourcePath);
		}

		Ref<Shader> shader;
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				shader = CreateRef<OpenGLShader>(std::move(resourcePath), bIsReload);
				break;
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}

		shader->Deserialize();
		if (bIsReload)
		{
			shader->m_OnShaderReloadedDel.publish();
		}
		return shader;
	}

	Ref<Shader> Shader::Create(std::string ID, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLShader>(std::move(ID), vertexSrc, fragmentSrc);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	void Shader::ClearCache(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				break;
			case RendererAPI::API::OpenGL:
			{
				const std::filesystem::path cacheDirectory = OpenGLShader::GetCacheDirectory();
				const std::filesystem::path shaderFilePath = path;
				const auto& extensions = OpenGLShader::GetCacheFileExtensions();
				for (const char* extension : extensions)
				{
					const std::string cachePath = (cacheDirectory / (shaderFilePath.filename().string() + extension)).string();
					if (PathUtils::DoesPathExist(cachePath))
					{
						PathUtils::DeletePath(cachePath);
					}
				}
				break;
			}
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				break;
		}
	}

	void Shader::Serialize(const std::string& path)
	{
		std::string assetPath = PathUtils::GetNormalizedAssetPath(path);
		if (!PathUtils::DoesPathExist(assetPath)) return;

		SetID(std::move(assetPath));
		AssetSerializer::Serialize(GetID(), TypeId(), {});
	}

	void Shader::Deserialize()
	{
		if (!PathUtils::DoesPathExist(GetID())) return;

		AssetSerializer::Deserialize(GetID(), TypeId(), {});
	}

}