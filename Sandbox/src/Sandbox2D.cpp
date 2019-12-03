#include "Sandbox2D.h"

#include <Engine/Core/EntryPoint.h>

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D")
	, m_CameraController(1280.f / 720.f)
{
}

void Sandbox2D::OnAttach()
{
	m_CheckerboardTexture = ZeoEngine::Texture2D::Create("assets/textures/Checkerboard_Alpha.png");
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(ZeoEngine::DeltaTime dt)
{
	// Update
	m_CameraController.OnUpdate(dt);

	// Render
	ZeoEngine::RenderCommand::SetClearColor({ 0.f, 0.f, 0.f, 1.f });
	ZeoEngine::RenderCommand::Clear();

	ZeoEngine::Renderer2D::BeginScene(m_CameraController.GetCamera());

	ZeoEngine::Renderer2D::DrawQuad({ -0.75f, 0.0f }, { 0.5f, 0.5f }, { 0.1f, 0.2f, 0.3f, 1.f });
	ZeoEngine::Renderer2D::DrawQuad({ 0.75f, 0.0f }, { 1.0f, 1.0f }, { 0.3f, 0.2f, 0.1f, 1.f });
	ZeoEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_CheckerboardTexture);

	ZeoEngine::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("SquareColor", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(ZeoEngine::Event& event)
{
	m_CameraController.OnEvent(event);
}
