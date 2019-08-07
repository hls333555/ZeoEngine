#include "HBestEngine.h"

#include "imgui/imgui.h"

class ExampleLayer : public HBestEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
		, m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		m_VAO.reset(HBestEngine::VertexArray::Create());

		float vertices[] = {
			-0.5f, -0.5f, 0.f, 0.8f, 0.2f, 0.8f, 1.f,
			 0.5f, -0.5f, 0.f, 0.2f, 0.3f, 0.8f, 1.f,
			 0.f,   0.5f, 0.f, 0.8f, 0.8f, 0.2f, 1.f
		};

		std::shared_ptr<HBestEngine::VertexBuffer> VBO;
		VBO.reset(HBestEngine::VertexBuffer::Create(vertices, sizeof(vertices)));

		HBestEngine::BufferLayout layout = {
			{ HBestEngine::ShaderDataType::Float3, "a_Position" },
			{ HBestEngine::ShaderDataType::Float4, "a_Color" }
		};
		VBO->SetLayout(layout);
		m_VAO->AddVertexBuffer(VBO);

		uint32_t indices[] = {
			0, 1, 2
		};

		// Use shared_ptr here because VAO will reference it
		std::shared_ptr<HBestEngine::IndexBuffer> IBO;
		IBO.reset(HBestEngine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VAO->SetIndexBuffer(IBO);


		m_SquareVAO.reset(HBestEngine::VertexArray::Create());

		float squareVertices[] = {
			-0.75f, -0.75f, 0.f,
			 0.75f, -0.75f, 0.f,
			 0.75f,  0.75f, 0.f,
			-0.75f,  0.75f, 0.f
		};

		std::shared_ptr<HBestEngine::VertexBuffer> squareVBO;
		squareVBO.reset(HBestEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		HBestEngine::BufferLayout squareLayout = {
			{ HBestEngine::ShaderDataType::Float3, "a_Position" },
		};
		squareVBO->SetLayout(squareLayout);
		m_SquareVAO->AddVertexBuffer(squareVBO);

		uint32_t squareIndices[] = {
			0, 1, 2,
			2, 3, 0
		};

		std::shared_ptr<HBestEngine::IndexBuffer> squareIBO;
		squareIBO.reset(HBestEngine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVAO->SetIndexBuffer(squareIBO);

		const std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;

			out vec4 v_Color;
			
			void main()
			{
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.f);

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

		m_Shader.reset(new HBestEngine::Shader(vertexSrc, fragmentSrc));

		const std::string blueShaderVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			
			void main()
			{
				gl_Position = u_ViewProjection * vec4(a_Position, 1.f);
			}
		)";

		const std::string blueShaderFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;
			
			void main()
			{
				color = vec4(0.2f, 0.3f, 0.6f, 1.f);
			}
		)";

		m_BlueShader.reset(new HBestEngine::Shader(blueShaderVertexSrc, blueShaderFragmentSrc));
	}

	virtual void OnUpdate() override
	{
		//HBE_INFO("ExampleLayer::Update");

		//RenderCommand::SetClearColor({ 1.f, 0.f, 1.f, 1.f });
		HBestEngine::RenderCommand::Clear();

		float cameraMoveSpeed = 0.01f;
		float cameraRotationSpeed = 0.03f;
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_A))
		{
			m_Camera.SetPosition({ m_Camera.GetPosition().x - cameraMoveSpeed, m_Camera.GetPosition().y, 0.f });
		}
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_D))
		{
			m_Camera.SetPosition({ m_Camera.GetPosition().x + cameraMoveSpeed, m_Camera.GetPosition().y, 0.f });
		}
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_W))
		{
			m_Camera.SetPosition({ m_Camera.GetPosition().x, m_Camera.GetPosition().y + cameraMoveSpeed, 0.f });
		}
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_S))
		{
			m_Camera.SetPosition({ m_Camera.GetPosition().x, m_Camera.GetPosition().y - cameraMoveSpeed, 0.f });
		}
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_Q))
		{
			m_Camera.SetRotation(m_Camera.GetRotation() + cameraRotationSpeed);
		}
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_E))
		{
			m_Camera.SetRotation(m_Camera.GetRotation() - cameraRotationSpeed);
		}

		HBestEngine::Renderer::BeginScene(m_Camera);

		m_BlueShader->Bind();
		HBestEngine::Renderer::Submit(m_BlueShader, m_SquareVAO);
		m_Shader->Bind();
		HBestEngine::Renderer::Submit(m_Shader, m_VAO);

		HBestEngine::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{

	}

	virtual void OnEvent(HBestEngine::Event& event) override
	{
		//HBE_TRACE("{0}", event);
		
	}

private:
	std::shared_ptr<HBestEngine::VertexArray> m_VAO;
	std::shared_ptr<HBestEngine::Shader> m_Shader;

	std::shared_ptr<HBestEngine::VertexArray> m_SquareVAO;
	std::shared_ptr<HBestEngine::Shader> m_BlueShader;

	HBestEngine::OrthographicCamera m_Camera;

};

class SandBox : public HBestEngine::Application
{
public:
	SandBox()
	{
		PushLayer(new ExampleLayer());
	}

	~SandBox()
	{

	}

};

HBestEngine::Application* HBestEngine::CreateApplication()
{
	return new SandBox();
}
