#include "Level.h"

#include <imgui/imgui.h>

Level::~Level()
{
	for (auto* object : m_GameObjects)
	{
		delete object;
	}
}

void Level::Init()
{
	m_backgroundTexture = ZeoEngine::Texture2D::Create("assets/textures/Checkerboard_Alpha.png");

	SpawnGameObject<Player>({ 0.0f, m_LevelBounds.bottom + 1.0f });
}

void Level::OnUpdate(ZeoEngine::DeltaTime dt)
{
	for (auto* object : m_GameObjects)
	{
		if (object->IsActive())
		{
			object->OnUpdate(dt);
		}
	}
}

void Level::OnRender()
{
	ZeoEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 100.0f, 100.0f }, m_backgroundTexture, 50.f);

	for (auto* object : m_GameObjects)
	{
		if (object->IsActive())
		{
			object->OnRender();
		}
	}
}

void Level::OnImGuiRender()
{
	for (auto* object : m_GameObjects)
	{
		if (object->IsActive())
		{
			object->OnImGuiRender();
		}
	}

	ImGui::Begin("Level Outline");

	for (auto* object : m_GameObjects)
	{
		ImGui::Text("%s", object->GetName().c_str());
	}

	ImGui::End();

	ImGui::Begin("Debug");

	ImGui::Text("%.f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate);

	ImGui::End();
	
}

void Level::DestroyGameObject(GameObject* object)
{
	auto it = std::find(m_GameObjects.begin(), m_GameObjects.end(), object);
	m_GameObjects.erase(it);
}
