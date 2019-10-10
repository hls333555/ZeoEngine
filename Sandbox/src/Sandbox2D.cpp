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
	m_SquareVAO = ZeoEngine::VertexArray::Create();

	float squareVertices[] = {
		-0.5f, -0.5f, 0.f,
		 0.5f, -0.5f, 0.f,
		 0.5f,  0.5f, 0.f,
		-0.5f,  0.5f, 0.f
	};

	ZeoEngine::Ref<ZeoEngine::VertexBuffer> squareVBO;
	squareVBO.reset(ZeoEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

	ZeoEngine::BufferLayout squareLayout = {
		{ ZeoEngine::ShaderDataType::Float3, "a_Position" },
	};
	squareVBO->SetLayout(squareLayout);
	m_SquareVAO->AddVertexBuffer(squareVBO);

	uint32_t squareIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

	ZeoEngine::Ref<ZeoEngine::IndexBuffer> squareIBO;
	squareIBO.reset(ZeoEngine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
	m_SquareVAO->SetIndexBuffer(squareIBO);

	m_FlatColorShader = ZeoEngine::Shader::Create("assets/shaders/FlatColor.glsl");

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

	ZeoEngine::Renderer::BeginScene(m_CameraController.GetCamera());

	static glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.1f));

	m_FlatColorShader->Bind();
	std::dynamic_pointer_cast<ZeoEngine::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);

	ZeoEngine::Renderer::Submit(m_FlatColorShader, m_SquareVAO);

	ZeoEngine::Renderer::EndScene();
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
