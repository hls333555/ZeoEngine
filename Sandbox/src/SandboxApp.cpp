#include "ZeoEngine.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Core/EntryPoint.h"

#include "Engine/Layers/EditorLayer.h"
#include "Engine/Layers/GameLayer.h"

class ExampleLayer : public ZeoEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
		, m_CameraController(1280.0f / 720.0f)
	{
		m_VAO = ZeoEngine::VertexArray::Create();

		float vertices[] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};

		ZeoEngine::Ref<ZeoEngine::VertexBuffer> VBO = ZeoEngine::VertexBuffer::Create(vertices, sizeof(vertices));

		ZeoEngine::BufferLayout layout = {
			{ ZeoEngine::ShaderDataType::Float3, "a_Position" },
			{ ZeoEngine::ShaderDataType::Float4, "a_Color" }
		};
		VBO->SetLayout(layout);
		m_VAO->AddVertexBuffer(VBO);

		uint32_t indices[] = {
			0, 1, 2
		};

		// Use shared_ptr here because VAO will reference it
		ZeoEngine::Ref<ZeoEngine::IndexBuffer> IBO = ZeoEngine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		m_VAO->SetIndexBuffer(IBO);


		m_SquareVAO = ZeoEngine::VertexArray::Create();

		float squareVertices[] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		ZeoEngine::Ref<ZeoEngine::VertexBuffer> squareVBO = ZeoEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices));

		ZeoEngine::BufferLayout squareLayout = {
			{ ZeoEngine::ShaderDataType::Float3, "a_Position" },
			{ ZeoEngine::ShaderDataType::Float2, "a_TexCoord" },
		};
		squareVBO->SetLayout(squareLayout);
		m_SquareVAO->AddVertexBuffer(squareVBO);

		uint32_t squareIndices[] = {
			0, 1, 2,
			2, 3, 0
		};

		ZeoEngine::Ref<ZeoEngine::IndexBuffer> squareIBO = ZeoEngine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		m_SquareVAO->SetIndexBuffer(squareIBO);

		const std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec4 v_Color;
			
			void main()
			{
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);

			}
		)";

		const std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;
			
			in vec4 v_Color;
			
			void main()
			{
				color = v_Color;
			}
		)";

		m_Shader = ZeoEngine::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

		const std::string flatColorShaderVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			
			void main()
			{
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
			}
		)";

		const std::string flatColorShaderFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			uniform vec4 u_Color;
			
			void main()
			{
				color = u_Color;
			}
		)";

		m_FlatColorShader = ZeoEngine::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = ZeoEngine::Texture2D::Create("assets/textures/Checkerboard_Alpha.png");
		m_LogoTexture = ZeoEngine::Texture2D::Create("assets/textures/Logo_Trans_D.png");
		
		textureShader->Bind();
		textureShader->SetInt("u_Texture", 0);

	}

	virtual void OnUpdate(ZeoEngine::DeltaTime dt) override
	{
		//ZE_INFO("ExampleLayer::Update");
		//ZE_INFO("Delta time: {0}s, {1}ms", dt.GetSeconds(), dt.GetMilliseconds());

		// Update
		m_CameraController.OnUpdate(dt);

		// Render
		//ZeoEngine::RenderCommand::SetClearColor({ 1.0f, 0.0f, 1.0f, 1.0f });
		ZeoEngine::RenderCommand::Clear();

		ZeoEngine::Renderer::BeginScene(m_CameraController.GetCamera());

		static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		m_FlatColorShader->Bind();
		m_FlatColorShader->SetFloat4("u_Color", m_SquareColor);

		for (int32_t x = 0; x < 10; ++x)
		{
			for (int32_t y = 0; y < 10; ++y)
			{
				glm::vec3 pos(x * 0.11f - 5 * 0.11f + 0.055f, y * 0.11f - 5 * 0.11f + 0.055f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				ZeoEngine::Renderer::Submit(m_FlatColorShader, m_SquareVAO, transform);
			}		
		}

		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();
		ZeoEngine::Renderer::Submit(textureShader, m_SquareVAO);

		m_LogoTexture->Bind();
		ZeoEngine::Renderer::Submit(textureShader, m_SquareVAO);

		// Triangle
		//ZeoEngine::Renderer::Submit(m_Shader, m_VAO);

		ZeoEngine::Renderer::EndScene();

	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit4("SquareColor", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	virtual void OnEvent(ZeoEngine::Event& event) override
	{
		//ZE_TRACE("{0}", event);
		
		m_CameraController.OnEvent(event);
	}

private:
	ZeoEngine::ShaderLibrary m_ShaderLibrary;

	ZeoEngine::Ref<ZeoEngine::VertexArray> m_VAO;
	ZeoEngine::Ref<ZeoEngine::Shader> m_Shader;

	ZeoEngine::Ref<ZeoEngine::VertexArray> m_SquareVAO;
	ZeoEngine::Ref<ZeoEngine::Shader> m_FlatColorShader;

	ZeoEngine::Ref<ZeoEngine::Texture2D> m_Texture;
	ZeoEngine::Ref<ZeoEngine::Texture2D> m_LogoTexture;

	ZeoEngine::OrthographicCameraController m_CameraController;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

};

class SandBox : public ZeoEngine::Application
{
public:
	SandBox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new ZeoEngine::EditorLayer());
		PushLayer(new ZeoEngine::GameLayer());
	}

	~SandBox()
	{
	}

};

ZeoEngine::Application* ZeoEngine::CreateApplication()
{
	return new SandBox();
}
