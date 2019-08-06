#include "HBEpch.h"
#include "Application.h"

#include "Log.h"

#include <glad/glad.h>

#include "Input.h"

namespace HBestEngine
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		HBE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(HBE_BIND_EVENT_FUNC(Application::OnEvent));
		
		// m_ImGuiLayer does not need to be unique pointer
		// since it is going to be part of the layer stack who will control its lifecycle
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		m_VAO.reset(VertexArray::Create());

		float vertices[] = {
			-0.5f, -0.5f, 0.f, 0.8f, 0.2f, 0.8f, 1.f,
			 0.5f, -0.5f, 0.f, 0.2f, 0.3f, 0.8f, 1.f,
			 0.f,   0.5f, 0.f, 0.8f, 0.8f, 0.2f, 1.f
		};

		std::shared_ptr<VertexBuffer> VBO;
		VBO.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" }
		};
		VBO->SetLayout(layout);
		m_VAO->AddVertexBuffer(VBO);
	
		uint32_t indices[] = {
			0, 1, 2
		};

		std::shared_ptr<IndexBuffer> IBO;
		IBO.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VAO->SetIndexBuffer(IBO);


		m_SquareVAO.reset(VertexArray::Create());

		float squareVertices[] = {
			-0.75f, -0.75f, 0.f,
			 0.75f, -0.75f, 0.f,
			 0.75f,  0.75f, 0.f,
			-0.75f,  0.75f, 0.f
		};

		std::shared_ptr<VertexBuffer> squareVBO;
		squareVBO.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		BufferLayout squareLayout = {
			{ ShaderDataType::Float3, "a_Position" },
		};
		squareVBO->SetLayout(squareLayout);
		m_SquareVAO->AddVertexBuffer(squareVBO);

		uint32_t squareIndices[] = {
			0, 1, 2,
			2, 3, 0
		};

		std::shared_ptr<IndexBuffer> squareIBO;
		squareIBO.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVAO->SetIndexBuffer(squareIBO);

		const std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec4 v_Color;
			
			void main()
			{
				v_Color = a_Color;
				gl_Position = vec4(a_Position, 1.f);
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

		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));

		const std::string blueShaderVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			
			void main()
			{
				gl_Position = vec4(a_Position, 1.f);
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

		m_BlueShader.reset(new Shader(blueShaderVertexSrc, blueShaderFragmentSrc));
	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(HBE_BIND_EVENT_FUNC(Application::OnWindowClose));

		// Iterate through the layer stack in a reverse order (from top to bottom) and break if current event is handled
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.m_bHandled)
				break;
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::Run()
	{
		while (m_bRunning)
		{
			//glClearColor(1, 0, 1, 1);
			// Called before any rendering calls!
			glClear(GL_COLOR_BUFFER_BIT);
			
			m_BlueShader->Bind();
			m_SquareVAO->Bind();
			glDrawElements(GL_TRIANGLES, m_SquareVAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			m_Shader->Bind();
			m_VAO->Bind();
			glDrawElements(GL_TRIANGLES, m_VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
			
			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate();
			}

			// TODO: This will eventually be in render thread
			// Render ImGui
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
			{
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_bRunning = false;
		return true;
	}

}

