#include "ZEpch.h"
#include "Engine/GameFramework/Scene.h"

#include "Engine/GameFramework/Entity.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Renderer/Renderer2D.h"

namespace ZeoEngine {

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{

	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<TransformComponent>();
		auto& tagComp = entity.AddComponent<TagComponent>();
		tagComp.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	void Scene::OnUpdate(DeltaTime dt)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			if (sprite.Texture)
			{
				Renderer2D::DrawRotatedQuad(transform, sprite.Texture, sprite.TextureTiling, { 0.0f, 0.0f }, sprite.TintColor);
			}
			else
			{
				Renderer2D::DrawRotatedQuad(transform, sprite.TintColor);
			}
		}
	}

}
