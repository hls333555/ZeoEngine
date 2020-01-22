#include "Level.h"

void Level::Init()
{
	m_CheckerboardTexture = ZeoEngine::Texture2D::Create("assets/textures/Checkerboard_Alpha.png");

	m_Player.Init(this);
}

void Level::OnUpdate(ZeoEngine::DeltaTime dt)
{
	m_Player.OnUpdate(dt);


}

void Level::OnRender()
{
	ZeoEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 100.0f, 100.0f }, m_CheckerboardTexture, 50.f);

	m_Player.OnRender();
}

void Level::OnImGuiRender()
{
	m_Player.OnImGuiRender();


}
