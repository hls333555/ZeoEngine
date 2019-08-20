#include "HBestEngine.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

class ExampleLayer : public HBestEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
		, m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
		, m_CameraPos(0.f)
	{
		m_VAO.reset(HBestEngine::VertexArray::Create());

		float vertices[] = {
			-0.5f, -0.5f, 0.f, 0.8f, 0.2f, 0.8f, 1.f,
			 0.5f, -0.5f, 0.f, 0.2f, 0.3f, 0.8f, 1.f,
			 0.f,   0.5f, 0.f, 0.8f, 0.8f, 0.2f, 1.f
		};

		HBestEngine::Ref<HBestEngine::VertexBuffer> VBO;
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
		HBestEngine::Ref<HBestEngine::IndexBuffer> IBO;
		IBO.reset(HBestEngine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VAO->SetIndexBuffer(IBO);


		m_SquareVAO.reset(HBestEngine::VertexArray::Create());

		float squareVertices[] = {
			-0.5f, -0.5f, 0.f, 0.f, 0.f,
			 0.5f, -0.5f, 0.f, 1.f, 0.f,
			 0.5f,  0.5f, 0.f, 1.f, 1.f,
			-0.5f,  0.5f, 0.f, 0.f, 1.f
		};

		HBestEngine::Ref<HBestEngine::VertexBuffer> squareVBO;
		squareVBO.reset(HBestEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		HBestEngine::BufferLayout squareLayout = {
			{ HBestEngine::ShaderDataType::Float3, "a_Position" },
			{ HBestEngine::ShaderDataType::Float2, "a_TexCoord" },
		};
		squareVBO->SetLayout(squareLayout);
		m_SquareVAO->AddVertexBuffer(squareVBO);

		uint32_t squareIndices[] = {
			0, 1, 2,
			2, 3, 0
		};

		HBestEngine::Ref<HBestEngine::IndexBuffer> squareIBO;
		squareIBO.reset(HBestEngine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
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
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.f);

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

		m_Shader.reset(HBestEngine::Shader::Create(vertexSrc, fragmentSrc));

		const std::string flatColorShaderVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			
			void main()
			{
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.f);
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

		m_FlatColorShader.reset(HBestEngine::Shader::Create(flatColorShaderVertexSrc, flatColorShaderFragmentSrc));

		const std::string textureShaderVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TexCoord;

			out vec2 v_TexCoord;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			
			void main()
			{
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.f);
				v_TexCoord = a_TexCoord;
			}
		)";

		const std::string textureShaderFragmentSrc = R"(
			#version 330 core

			in vec2 v_TexCoord;

			layout(location = 0) out vec4 color;

			uniform sampler2D u_Texture;
			
			void main()
			{
				color = texture(u_Texture, v_TexCoord);
			}
		)";

		m_TextureShader.reset(HBestEngine::Shader::Create(textureShaderVertexSrc, textureShaderFragmentSrc));

		m_Texture = HBestEngine::Texture2D::Create("assets/textures/Checkerboard.png");
		
		m_TextureShader->Bind();
		std::dynamic_pointer_cast<HBestEngine::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);

	}

	virtual void OnUpdate(HBestEngine::DeltaTime dt) override
	{
		//HBE_INFO("ExampleLayer::Update");
		//HBE_INFO("Delta time: {0}s, {1}ms", dt.GetSeconds(), dt.GetMilliseconds());

		//RenderCommand::SetClearColor({ 1.f, 0.f, 1.f, 1.f });
		HBestEngine::RenderCommand::Clear();

		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_A))
		{
			m_CameraPos.x -= m_CameraMoveSpeed * dt;
		}
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_D))
		{
			m_CameraPos.x += m_CameraMoveSpeed * dt;
		}
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_W))
		{
			m_CameraPos.y += m_CameraMoveSpeed * dt;
		}
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_S))
		{
			m_CameraPos.y -= m_CameraMoveSpeed * dt;
		}
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_Q))
		{
			m_CameraRot -= m_CameraRotateSpeed * dt;
		}
		if (HBestEngine::Input::IsKeyPressed(HBE_KEY_E))
		{
			m_CameraRot += m_CameraRotateSpeed * dt;
		}

		m_Camera.SetPosition(m_CameraPos);
		m_Camera.SetRotation(m_CameraRot);

		HBestEngine::Renderer::BeginScene(m_Camera);

		static glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.1f));

		m_FlatColorShader->Bind();
		std::dynamic_pointer_cast<HBestEngine::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);

		for (int x = 0; x < 10; ++x)
		{
			for (int y = 0; y < 10; ++y)
			{
				glm::vec3 pos(x * 0.11f - 5 * 0.11f + 0.055f, y * 0.11f - 5 * 0.11f + 0.055f, 0.f);
				glm::mat4 transform = glm::translate(glm::mat4(1.f), pos) * scale;
				HBestEngine::Renderer::Submit(m_FlatColorShader, m_SquareVAO, transform);
			}		
		}

		m_Texture->Bind();
		HBestEngine::Renderer::Submit(m_TextureShader, m_SquareVAO);

		// Triangle
		//HBestEngine::Renderer::Submit(m_Shader, m_VAO);

		HBestEngine::Renderer::EndScene();

	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit4("SquareColor", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	virtual void OnEvent(HBestEngine::Event& event) override
	{
		//HBE_TRACE("{0}", event);
		
	}

private:
	HBestEngine::Ref<HBestEngine::VertexArray> m_VAO;
	HBestEngine::Ref<HBestEngine::Shader> m_Shader;

	HBestEngine::Ref<HBestEngine::VertexArray> m_SquareVAO;
	HBestEngine::Ref<HBestEngine::Shader> m_FlatColorShader, m_TextureShader;

	HBestEngine::Ref<HBestEngine::Texture2D> m_Texture;

	HBestEngine::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPos;
	float m_CameraRot = 0.f;
	float m_CameraMoveSpeed = 5.f;
	/** How many degrees to rotate per second */
	float m_CameraRotateSpeed = 180.f;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.f };

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
