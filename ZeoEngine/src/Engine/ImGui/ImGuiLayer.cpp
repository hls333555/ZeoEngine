#include "ZEpch.h"
#include "Engine/ImGui/ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <IconsFontAwesome5.h>

#include "Engine/Core/Application.h"
#include "Engine/Renderer/Renderer.h"

// TEMPORARY
#include <GLFW/glfw3.h>
#include <glad/glad.h>

const char* glsl_version = "#version 410";

namespace ZeoEngine {

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGui")
	{
	}

	struct PreCallbackData
	{
		// Chain GLFW callbacks: our callbacks will call the ImGui previously installed callbacks
		GLFWwindowsizefun       PrevUserCallbackWindowSize;
		GLFWwindowclosefun      PrevUserCallbackWindowClose;
		GLFWwindowfocusfun      PrevUserCallbackWindowFocus;
		GLFWdropfun				PrevUserCallbackWindowFileDrop;
		GLFWkeyfun              PrevUserCallbackKey;
		GLFWcharfun             PrevUserCallbackChar;
		GLFWmousebuttonfun      PrevUserCallbackMousebutton;
		GLFWscrollfun           PrevUserCallbackScroll;
		GLFWcursorposfun        PrevUserCallbackCursorPos;
	};

	// https://github.com/ocornut/imgui/issues/5397
	static void (*OldCreateWindow)(ImGuiViewport*);
	static void (*OldDestroyWindow)(ImGuiViewport*);

	static void ImGui_ImplGlfw_CreateWindow(ImGuiViewport* viewport)
	{
		OldCreateWindow(viewport);

		auto* window = static_cast<GLFWwindow*>(viewport->PlatformHandle);
		Window::SetIcon(window);

		Application::Get().AddViewportWindow(window);

		// Enable raw mouse motion if supported
		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		// ---Set GLFW callbacks------------------------------------------------------------------------------------

		auto* data = new PreCallbackData();
		glfwSetWindowUserPointer(window, data);

		data->PrevUserCallbackWindowSize = glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
			const auto* data = static_cast<PreCallbackData*>(glfwGetWindowUserPointer(window));
			if (data->PrevUserCallbackWindowSize)
			{
				data->PrevUserCallbackWindowSize(window, width, height);
			}
			WindowResizeEvent event(window, width, height);
			Application::Get().OnEvent(event);
		});

		data->PrevUserCallbackWindowClose = glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
			const auto* data = static_cast<PreCallbackData*>(glfwGetWindowUserPointer(window));
			if (data->PrevUserCallbackWindowClose)
			{
				data->PrevUserCallbackWindowClose(window);
			}
			WindowCloseEvent event(window);
			Application::Get().OnEvent(event);
		});

		data->PrevUserCallbackWindowFocus = glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int bFocused) {
			const auto* data = static_cast<PreCallbackData*>(glfwGetWindowUserPointer(window));
			if (data->PrevUserCallbackWindowFocus)
			{
				data->PrevUserCallbackWindowFocus(window, bFocused);
			}
			WindowFocusChangedEvent event(window, bFocused == GLFW_TRUE);
			Application::Get().OnEvent(event);
		});

		data->PrevUserCallbackWindowFileDrop = glfwSetDropCallback(window, [](GLFWwindow* window, int count, const char** paths) {
			const auto* data = static_cast<PreCallbackData*>(glfwGetWindowUserPointer(window));
			if (data->PrevUserCallbackWindowFileDrop)
			{
				data->PrevUserCallbackWindowFileDrop(window, count, paths);
			}
			WindowFileDroppedEvent event(window, count, paths);
			Application::Get().OnEvent(event);
		});

		data->PrevUserCallbackKey = glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			const auto* data = static_cast<PreCallbackData*>(glfwGetWindowUserPointer(window));
			if (data->PrevUserCallbackKey)
			{
				data->PrevUserCallbackKey(window, key, scancode, action, mods);
			}
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(window, key, 0);
				Application::Get().OnEvent(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(window, key);
				Application::Get().OnEvent(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(window, key, 1);
				Application::Get().OnEvent(event);
				break;
			}
			default:
				break;
			}
		});

		data->PrevUserCallbackChar = glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int key) {
			const auto* data = static_cast<PreCallbackData*>(glfwGetWindowUserPointer(window));
			if (data->PrevUserCallbackChar)
			{
				data->PrevUserCallbackChar(window, key);
			}
			KeyTypedEvent event(window, key);
			Application::Get().OnEvent(event);
		});

		data->PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
			const auto* data = static_cast<PreCallbackData*>(glfwGetWindowUserPointer(window));
			if (data->PrevUserCallbackMousebutton)
			{
				data->PrevUserCallbackMousebutton(window, button, action, mods);
			}
			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(window, button);
				Application::Get().OnEvent(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(window, button);
				Application::Get().OnEvent(event);
				break;
			}
			default:
				break;
			}
		});

		data->PrevUserCallbackScroll = glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
			const auto* data = static_cast<PreCallbackData*>(glfwGetWindowUserPointer(window));
			if (data->PrevUserCallbackScroll)
			{
				data->PrevUserCallbackScroll(window, xoffset, yoffset);
			}
			MouseScrolledEvent event(window, static_cast<float>(xoffset), static_cast<float>(yoffset));
			Application::Get().OnEvent(event);
		});

		data->PrevUserCallbackCursorPos = glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
			const auto* data = static_cast<PreCallbackData*>(glfwGetWindowUserPointer(window));
			if (data->PrevUserCallbackCursorPos)
			{
				data->PrevUserCallbackCursorPos(window, xpos, ypos);
			}
			MouseMovedEvent event(window, static_cast<float>(xpos), static_cast<float>(ypos));
			Application::Get().OnEvent(event);
		});

		Renderer::Init();
	}

	static void ImGui_ImplGlfw_DestroyWindow(ImGuiViewport* viewport)
	{
		// Main viewport does not set this data
		if (viewport != ImGui::GetMainViewport())
		{
			auto* window = static_cast<GLFWwindow*>(viewport->PlatformHandle);
			Application::Get().RemoveViewportWindow(window);
			auto* data = glfwGetWindowUserPointer(window);
			delete static_cast<PreCallbackData*>(data);
		}
		OldDestroyWindow(viewport);
	}

	static void ImGui_ImplGlfw_InitPlatformInterface()
	{
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		OldCreateWindow = platformIO.Platform_CreateWindow;
		OldDestroyWindow = platformIO.Platform_DestroyWindow;
		platformIO.Platform_CreateWindow = ImGui_ImplGlfw_CreateWindow;
		platformIO.Platform_DestroyWindow = ImGui_ImplGlfw_DestroyWindow;
	}

	static void ImGui_ImplOpenGL3_RenderWindow(ImGuiViewport* viewport, void*)
	{
		ImGui_ImplOpenGL3_RenderDrawData(viewport->DrawData);
	}

	static void ImGui_ImplOpenGL3_InitPlatformInterface()
	{
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		platformIO.Renderer_RenderWindow = ImGui_ImplOpenGL3_RenderWindow;
	}

	void ImGuiLayer::OnAttach()
	{
		ZE_PROFILE_FUNCTION();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		io.ConfigWindowsMoveFromTitleBarOnly = true;				// Disable moving from window body

		LoadDefaultFont();

		// Setup dark Dear ImGui style
		ImGui::StyleColorsDark();
		SetDarkThemeColors();

		// Set default style
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 8.0f;
		style.WindowBorderSize = 0.0f;
		style.PopupRounding = 5.0f;
		style.PopupBorderSize = 0.0f;
		style.FrameRounding = 5.0f;

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui_ImplGlfw_InitPlatformInterface();
			ImGui_ImplOpenGL3_InitPlatformInterface();
		}
	}

	void ImGuiLayer::OnDetach()
	{
		ZE_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		// TODO: Not used for now
		if (m_bBlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.m_bHandled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.m_bHandled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::Begin()
	{
		ZE_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		ZE_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		const auto& window = Application::Get().GetWindow();
		io.DisplaySize = ImVec2(static_cast<float>(window.GetWidth()), static_cast<float>(window.GetHeight()));

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::LoadDefaultFont()
	{
		ImGuiIO& io = ImGui::GetIO();

		// Load full Chinese characters
		io.Fonts->AddFontFromFileTTF("resources/fonts/wqy-microhei.ttc", 15.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

		ImFontConfig config;
		config.MergeMode = true; // Merge into first font
		config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
		static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		// Load font awesome 5 icons
		io.Fonts->AddFontFromFileTTF("resources/fonts/" FONT_ICON_FILE_NAME_FAS, 13.0f, &config, iconRanges);

		io.Fonts->Build();
	}

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.13f, 0.135f, 0.14f, 1.0f };
		colors[ImGuiCol_PopupBg] = ImVec4{ 0.05f, 0.05f, 0.05f, 1.0f };
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4{ 0.13f, 0.13f, 0.13f, 0.8f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}

}
