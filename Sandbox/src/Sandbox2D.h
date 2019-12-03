#pragma once

#include "ZeoEngine.h"

class Sandbox2D : public ZeoEngine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(ZeoEngine::Event& event) override;

private:
	ZeoEngine::OrthographicCameraController m_CameraController;

	ZeoEngine::Ref<ZeoEngine::VertexArray> m_SquareVAO;
	ZeoEngine::Ref<ZeoEngine::Shader> m_FlatColorShader;

	ZeoEngine::Ref<ZeoEngine::Texture2D> m_CheckerboardTexture;
	
	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};
