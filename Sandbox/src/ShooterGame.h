#pragma once

#include "ZeoEngine.h"

#include "Level.h"
#include "TimerManager.h"

struct ImFont;

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

	inline TimerManager* GetTimerManager() { return &m_TimerManager; }
	inline Level* GetLevel() { return &m_Level; }
	inline ImFont* GetFont() { return m_Font; }

private:
	bool OnWindowResized(ZeoEngine::WindowResizeEvent& e);
	void CreateCamera(uint32_t width, uint32_t height);

private:
	ZeoEngine::Scope<ZeoEngine::OrthographicCamera> m_Camera;
	TimerManager m_TimerManager;
	Level m_Level;
	ImFont* m_Font;

};

static Level* GetLevel()
{
	ShooterGame* gameLayer = ZeoEngine::Application::Get().FindLayerByName<ShooterGame>("Game");
	return gameLayer ? gameLayer->GetLevel() : nullptr;
}

static TimerManager* GetTimerManager()
{
	ShooterGame* gameLayer = ZeoEngine::Application::Get().FindLayerByName<ShooterGame>("Game");
	return gameLayer ? gameLayer->GetTimerManager() : nullptr;
}

static ImFont* GetFont()
{
	ShooterGame* gameLayer = ZeoEngine::Application::Get().FindLayerByName<ShooterGame>("Game");
	return gameLayer ? gameLayer->GetFont() : nullptr;
}