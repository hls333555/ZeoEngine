#include "ZEpch.h"
#include "Engine/Renderer/Shader.h"

#include "Engine/Asset/AssetLibrary.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Utils/PathUtils.h"

namespace ZeoEngine {
	
	Ref<Shader> Shader::Create(std::string resourcePath)
	{
		Ref<Shader> shader;
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				shader = CreateRef<OpenGLShader>(std::move(resourcePath));
				break;
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}

		shader->ParseAndCompile();
		return shader;
	}

	Ref<Shader> Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLShader>(vertexSrc, fragmentSrc);
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

	void Shader::ClearCache(const std::filesystem::path& path)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				break;
			case RendererAPI::API::OpenGL:
			{
				const std::filesystem::path cacheDirectory = OpenGLShader::GetCacheDirectory();
				const auto& extensions = OpenGLShader::GetCacheFileExtensions();
				for (const char* extension : extensions)
				{
					auto filename = path.filename();
					filename += extension;
					const auto cachePath = cacheDirectory / filename;
					if (PathUtils::Exists(cachePath))
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

	Ref<Shader> Shader::GetDefaultShader()
	{
		return AssetLibrary::LoadAsset<Shader>("assets/editor/shaders/Default.glsl.zasset");
	}

	void Shader::Reload()
	{
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(GetHandle());
		ClearCache(metadata->GetResourcePath());
		Timer timer;
		if (ParseAndCompile())
		{
			ZE_CORE_WARN("Reloading shader {0} took {1} ms", metadata->Path, timer.ElapsedMillis());
		}
	}

}
