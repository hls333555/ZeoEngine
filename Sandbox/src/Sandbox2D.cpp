#include "Sandbox2D.h"

#include <Engine/Core/EntryPoint.h>

#include "imgui/imgui.h"

#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D")
	, m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	ZE_PROFILE_FUNCTION();

	m_CheckerboardTexture = ZeoEngine::Texture2D::Create("assets/textures/Checkerboard_Alpha.png");
}

void Sandbox2D::OnDetach()
{
	ZE_PROFILE_FUNCTION();

}

void Sandbox2D::OnUpdate(ZeoEngine::DeltaTime dt)
{
	ZE_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(dt);

	// Render
	{
		ZE_PROFILE_SCOPE("Renderer Prep");

		ZeoEngine::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		ZeoEngine::RenderCommand::Clear();
	}

	{
		ZE_PROFILE_SCOPE("Renderer Draw");

		ZeoEngine::Renderer2D::BeginScene(m_CameraController.GetCamera());

		ZeoEngine::Renderer2D::DrawQuad({ -0.75f, 0.0f }, { 0.5f, 0.5f }, { 0.1f, 0.2f, 0.3f, 1.0f });
		ZeoEngine::Renderer2D::DrawQuad({ 0.75f, 0.0f }, { 1.0f, 1.0f }, { 0.3f, 0.2f, 0.1f, 1.0f });
		ZeoEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_CheckerboardTexture);

		ZeoEngine::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	ZE_PROFILE_FUNCTION();

	ImGui::Begin("Settings");

	ImGui::ColorEdit4("SquareColor", glm::value_ptr(m_SquareColor));

	ImGui::End();
}

void Sandbox2D::OnEvent(ZeoEngine::Event& event)
{
	m_CameraController.OnEvent(event);
}
