#include "ZEpch.h"
#include "Engine/Renderer/Shader.h"

#include "Engine/Asset/AssetLibrary.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Engine/Profile/BenchmarkTimer.h"

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
		Ref<Shader> shader;
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				ZE_CORE_ASSERT(false, "RendererAPI is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGL:
				shader = CreateRef<OpenGLShader>(vertexSrc, fragmentSrc);
				break;
			default:
				ZE_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
		// TODO:
		//shader->Compile();
		return shader;
	}

	Ref<ShaderInstance> Shader::CreateInstance()
	{
		return CreateRef<ShaderInstance>(SharedFromThis());
	}

	void Shader::Reload()
	{
		ClearCache();
		Timer timer;
		if (ParseAndCompile())
		{
			const float duration = timer.ElapsedMillis();
			const auto metadata = AssetRegistry::Get().GetAssetMetadata(GetHandle());
			ZE_CORE_WARN("Reloading shader \"{0}\" took {1} ms", metadata->Path, duration);
		}
	}

	void ShaderInstance::Bind() const
	{
		const auto shader = GetShader();
		shader->SetActiveRendererIDByID(m_ShaderVariantID);
		shader->Bind();
	}

	Ref<Shader> ShaderInstance::GetShader() const
	{
		return AssetLibrary::LoadAsset<Shader>(m_ShaderAsset);
	}

	void ShaderInstance::SetShaderVariantByMacro(const std::string& name, U32 value)
	{
		const auto variant = GetShader()->GetVariantByID(m_ShaderVariantID);
		ZE_CORE_ASSERT(variant);
		// Fetch current macro combination and update it with our value to generate the new variant ID
		ShaderVariantData tempData(variant->Data->Macros);
		tempData.SetMacro(name, std::to_string(value));
		m_ShaderVariantID = tempData.GetID();
	}

}
