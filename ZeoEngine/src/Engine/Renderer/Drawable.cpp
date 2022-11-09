#include "ZEpch.h"
#include "Engine/Renderer/Drawable.h"

#include "Engine/Asset/AssetLibrary.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/VertexArray.h"

namespace ZeoEngine {

	Drawable::Drawable(const Ref<VertexArray>& vao, const Ref<UniformBuffer>& ubo, const SceneContext* sceneContext)
		: m_VAO(vao), m_ModelUniformBuffer(ubo)
		, m_SceneContext(sceneContext)
	{
	}

	Drawable::~Drawable()
	{
		if (const auto material = AssetLibrary::LoadAsset<Material>(m_MaterialAsset))
		{
			material->m_OnMaterialInitialized.disconnect(this);
		}
	}

	void Drawable::SetMaterial(AssetHandle materialAsset)
	{
		if (materialAsset != m_MaterialAsset)
		{
			const auto material = AssetLibrary::LoadAsset<Material>(materialAsset);
			if (const auto lastMaterial = AssetLibrary::LoadAsset<Material>(m_MaterialAsset))
			{
				lastMaterial->m_OnMaterialInitialized.disconnect(this);
			}
			// Rebuild step instances after material being initialized as render step has been destroyed
			material->m_OnMaterialInitialized.connect<&Drawable::RebuildStepInstances>(this);
			// Rebuild step instances on material change
			RebuildStepInstances(*material);
			m_MaterialAsset = materialAsset;
		}
	}

	void Drawable::RebuildStepInstances(const Material& material)
	{
		m_StepInstances.clear();
		
		for (const auto& technique : material.GetRenderTechniques())
		{
			if (!technique.IsActive()) continue;

			for (const auto& step : technique.GetRenderSteps())
			{
				m_StepInstances.emplace_back(&step, m_SceneContext);
			}
		}
	}

	void Drawable::Bind() const
	{
		m_VAO->Bind();
		m_ModelUniformBuffer->Bind();
	}

	void Drawable::Submit(AssetHandle materialAsset)
	{
		SetMaterial(materialAsset);
		for (const auto& stepInstance : m_StepInstances)
		{
			stepInstance.Submit(*this);
		}
	}

}
