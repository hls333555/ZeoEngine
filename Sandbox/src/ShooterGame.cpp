#include "ShooterGame.h"

#include "RandomEngine.h"

ShooterGame::ShooterGame()
	: Layer("Game")
{
	const auto& window = ZeoEngine::Application::Get().GetWindow();
	CreateCamera(window.GetWidth(), window.GetHeight());

	RandomEngine::Init();
}

void ShooterGame::OnAttach()
{
	ZE_PROFILE_FUNCTION();

	m_Level.Init();
}

void ShooterGame::OnDetach()
{
	ZE_PROFILE_FUNCTION();

}

void ShooterGame::OnUpdate(ZeoEngine::DeltaTime dt)
{
	ZE_PROFILE_FUNCTION();

	m_TimerManager.OnUpdate(dt);
	m_Level.OnUpdate(dt);

	// Render
	{
		ZE_PROFILE_SCOPE("Renderer Prep");

		ZeoEngine::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		ZeoEngine::RenderCommand::Clear();
	}

	{
		ZE_PROFILE_SCOPE("Renderer Draw");

		ZeoEngine::Renderer2D::BeginScene(*m_Camera);

		m_Level.OnRender();

		ZeoEngine::Renderer2D::EndScene();
	}
}

void ShooterGame::OnImGuiRender()
{
	ZE_PROFILE_FUNCTION();

	m_Level.OnImGuiRender();
}

void ShooterGame::OnEvent(ZeoEngine::Event& event)
{
	ZE_PROFILE_FUNCTION();

	ZeoEngine::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<ZeoEngine::WindowResizeEvent>(ZE_BIND_EVENT_FUNC(ShooterGame::OnWindowResized));
}

bool ShooterGame::OnWindowResized(ZeoEngine::WindowResizeEvent& e)
{
	ZE_PROFILE_FUNCTION();

	CreateCamera(e.GetWidth(), e.GetHeight());

	return false;
}

void ShooterGame::CreateCamera(uint32_t width, uint32_t height)
{
	float aspectRatio = (float)width / (float)height;
	float zoomLevel = 4.0f;
	float left = -zoomLevel * aspectRatio;
	float right = zoomLevel * aspectRatio;
	float bottom = -zoomLevel;
	float top = zoomLevel;
	m_Camera = ZeoEngine::CreateScope<ZeoEngine::OrthographicCamera>(left, right, bottom, top);
	m_Level.SetLevelBounds({ left, right, bottom, top });
}
