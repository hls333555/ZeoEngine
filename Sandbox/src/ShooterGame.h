#pragma once

#include "ZeoEngine.h"

#include "Level.h"

class ShooterGame : public ZeoEngine::Layer
{
public:
	ShooterGame();
	virtual ~ShooterGame() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(ZeoEngine::Event& event) override;

private:
	bool OnWindowResized(ZeoEngine::WindowResizeEvent& e);
	void CreateCamera(uint32_t width, uint32_t height);

private:
	ZeoEngine::Scope<ZeoEngine::OrthographicCamera> m_Camera;

	Level m_Level;

};