#include "ZEpch.h"
#include "Engine/Layers/EditorLayer.h"

#include <filesystem>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo.h>
#include <nfd.h>

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Core/Application.h"
#include "Engine/Layers/GameLayer.h"
#include "Engine/Core/EngineGlobals.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Core/EngineUtilities.h"

namespace ZeoEngine {

	PIEState g_PIEState;

	EditorLayer::EditorLayer()
		: Layer("Editor")
	{
		const auto& window = Application::Get().GetWindow();
		m_EditorCameraController = CreateScope<OrthographicCameraController>((float)window.GetWidth() / (float)window.GetHeight());
		m_EditorCameraController->SetZoomLevel(3.0f);
	}

	void EditorLayer::OnAttach()
	{
		ConstructClassInheritanceTree();
		LoadEditorTextures();

		std::string cachePath = std::filesystem::current_path().string().append("/temp");
		// Create temp folder on demand
		std::filesystem::create_directory(cachePath);
	}

	void EditorLayer::OnUpdate(DeltaTime dt)
	{
		if (g_PIEState == PIEState::None && m_bIsHoveringGameView)
		{
			// Thses camera controls are only applied to Game View window
			m_EditorCameraController->OnUpdate(dt);
		}

		Level::Get().OnEditorUpdate(dt);
	}

	void EditorLayer::OnImGuiRender()
	{
		bool bShow = false;
		ImGui::ShowDemoWindow(&bShow);

		// TODO: These bools are not saved out, so last opened windows cannot restore
		static bool bShowGameView = true;
		static bool bShowLevelOutline = true;
		static bool bShowObjectProperty = true;
		static bool bShowClassBrowser = true;
		static bool bShowConsole = true;
		static bool bShowParticleEditor = false;
		static bool bShowPreferences = false;
		static bool bShowAbout = false;

		//////////////////////////////////////////////////////////////////////////
		// EditorDockspace ///////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		ShowEditorDockspace();

		//////////////////////////////////////////////////////////////////////////
		// MainMenuBar ///////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		// TODO: Key shortcuts
		if (ImGui::BeginMainMenuBar())
		{
			// TODO: Test these if we have another window containing these named menus

			// File menu
			if (ImGui::BeginMenu("File"))
			{
				static bool bEnableLoadingOrSaving = true;
				// Loading and saving level during PIE is not allowed
				if (g_PIEState != PIEState::None)
				{
					bEnableLoadingOrSaving = false;
				}
				else
				{
					bEnableLoadingOrSaving = true;
				}
				if (ImGui::MenuItem("Open level", "CTRL+O", false, bEnableLoadingOrSaving))
				{
					nfdchar_t* outPath = nullptr;
					nfdresult_t result = NFD_OpenDialog("zlevel", nullptr, &outPath);
					if (result == NFD_OKAY)
					{
						Level::Get().LoadLevelFromFile(outPath);
						m_CurrentLevelPath = outPath;
						m_CurrentLevelName = std::filesystem::path(outPath).filename().string();
						free(outPath);
					}
					else if (result == NFD_ERROR)
					{
						ZE_CORE_ERROR("Open level failure: {0}", NFD_GetError());
					}
				}
				if (ImGui::MenuItem("Save level", "CTRL+S", false, bEnableLoadingOrSaving))
				{
					if (m_CurrentLevelPath.empty())
					{
						nfdchar_t* outPath = nullptr;
						nfdresult_t result = NFD_SaveDialog("zlevel", nullptr, &outPath);
						if (result == NFD_OKAY)
						{
							Level::Get().SaveLevelToFile(std::string(outPath));
							m_CurrentLevelPath = outPath;
							m_CurrentLevelName = std::filesystem::path(outPath).filename().string();
							free(outPath);
						}
						else if (result == NFD_ERROR)
						{
							ZE_CORE_ERROR("Save level failure: {0}", NFD_GetError());
						}
					}
					else
					{
						Level::Get().SaveLevelToFile(m_CurrentLevelPath);
					}
				}
				if (ImGui::MenuItem("Save level as", "CTRL+ALT+S", false, bEnableLoadingOrSaving))
				{
					nfdchar_t* outPath = nullptr;
					nfdresult_t result = NFD_SaveDialog("zlevel", nullptr, &outPath);
					if (result == NFD_OKAY)
					{
						Level::Get().SaveLevelToFile(std::string(outPath));
						m_CurrentLevelPath = outPath;
						m_CurrentLevelName = std::filesystem::path(outPath).filename().string();
						free(outPath);
					}
					else if (result == NFD_ERROR)
					{
						ZE_CORE_ERROR("Save level failure: {0}", NFD_GetError());
					}
				}
				ImGui::EndMenu();
			}
			// Edit menu
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z"))
				{

				}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false))
				{

				}
				ImGui::Separator();
				if (ImGui::MenuItem("Copy", "CTRL+C"))
				{

				}
				if (ImGui::MenuItem("Paste", "CTRL+V"))
				{

				}
				if (ImGui::MenuItem("Cut", "CTRL+X"))
				{

				}
				ImGui::Separator();
				ImGui::MenuItem("Preferences", nullptr, &bShowPreferences);
				ImGui::EndMenu();
			}
			// Window menu
			if (ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("Game View", nullptr, &bShowGameView);
				ImGui::MenuItem("Level Outline", nullptr, &bShowLevelOutline);
				ImGui::MenuItem("Object Property", nullptr, &bShowObjectProperty);
				ImGui::MenuItem("Class Browser", nullptr, &bShowClassBrowser);
				ImGui::MenuItem("Console", nullptr, &bShowConsole);
				ImGui::MenuItem("ParticleEditor", nullptr, &bShowParticleEditor);
				ImGui::Separator();
				// Reset layout on next frame
				if (ImGui::MenuItem("Reset layout", nullptr))
				{
					m_bResetLayout = true;
				}
				ImGui::EndMenu();
			}
			// Help
			if (ImGui::BeginMenu("Help"))
			{
				ImGui::Separator();
				ImGui::MenuItem("About ZeoEngine", nullptr, &bShowAbout);
				ImGui::EndMenu();
			}
			// Display level name at center of menu bar
			{
				ImGui::TextCentered("%s", m_CurrentLevelName.empty() ? "Untitled" : m_CurrentLevelName.c_str());
			}
			// Display engine stats at right corner of menu bar
			{
				const float statsWidth = 125.0f;
				ImGui::Indent(ImGui::GetWindowSize().x - statsWidth);
				ImGui::Text("%.f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate);
			}
			ImGui::EndMainMenuBar();
		}

		//////////////////////////////////////////////////////////////////////////
		// Editor Windows ////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		if (bShowGameView)
		{
			ShowGameView(&bShowGameView);
		}
		else
		{
			// TODO: Disable framebuffer rendering

		}
		if (bShowLevelOutline)
		{
			ShowLevelOutline(&bShowLevelOutline);
		}
		if (bShowObjectProperty)
		{
			ShowObjectProperty(&bShowObjectProperty);
		}
		if (bShowClassBrowser)
		{
			ShowClassBrowser(&bShowClassBrowser);
		}
		if (bShowConsole)
		{
			ShowConsole(&bShowConsole);
		}

		if (bShowParticleEditor)
		{
			ShowParticleEditor(&bShowParticleEditor);
		}

		if (bShowPreferences)
		{
			ShowPreferences(&bShowPreferences);
		}

		if (bShowAbout)
		{
			ShowAbout(&bShowAbout);
		}
	}

	void EditorLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(EditorLayer::OnKeyPressed));

		if (m_bIsHoveringGameView)
		{
			// Thses camera events are only applied to Game View window
			m_EditorCameraController->OnEvent(event);
		}

	}

	void EditorLayer::LoadEditorTextures()
	{
		m_ToolBarTextures[0] = m_PlayTexture = Texture2D::Create("editor_assets/textures/Play.png");
		m_ToolBarTextures[1] = m_PauseTexture = Texture2D::Create("editor_assets/textures/Pause.png");
		m_StopTexture = Texture2D::Create("editor_assets/textures/Stop.png");

		m_LogoTexture = Texture2D::Create("editor_assets/textures/Logo.png");

	}

	void EditorLayer::ShowEditorDockspace()
	{
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		// Setting a little padding here can display intact yellow highlight rectangle when hovering over docked windows (Game View) during drag and drop operations
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
		windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		static bool bOpen = true;
		ImGui::Begin("Editor", &bOpen, windowFlags);
		ImGui::PopStyleVar(3);
		ImGuiID editorDockspaceId = ImGui::GetID("EditorDockspace");
		// Update docking layout
		if (ImGui::DockBuilderGetNode(editorDockspaceId) == nullptr || m_bResetLayout)
		{
			m_bResetLayout = false;

			// Clear out existing layout
			ImGui::DockBuilderRemoveNode(editorDockspaceId);
			// Add empty node
			ImGui::DockBuilderAddNode(editorDockspaceId, ImGuiDockNodeFlags_DockSpace);
			const auto& window = Application::Get().GetWindow();
			// Main node should cover entire screen
			ImGui::DockBuilderSetNodeSize(editorDockspaceId, ImVec2((float)window.GetWidth(), (float)window.GetHeight()));

			ImGuiID dockMainLeft;
			ImGuiID dockMainRight = ImGui::DockBuilderSplitNode(editorDockspaceId, ImGuiDir_Right, 0.2f, nullptr, &dockMainLeft);
			ImGuiID dockRightDown;
			ImGuiID dockRightUp = ImGui::DockBuilderSplitNode(dockMainRight, ImGuiDir_Up, 0.4f, nullptr, &dockRightDown);
			ImGuiID dockMainLeftUp;
			ImGuiID dockMainLeftDown = ImGui::DockBuilderSplitNode(dockMainLeft, ImGuiDir_Down, 0.3f, nullptr, &dockMainLeftUp);
			ImGuiID dockMainLeftRight;
			ImGuiID dockMainLeftLeft = ImGui::DockBuilderSplitNode(dockMainLeftUp, ImGuiDir_Left, 0.2f, nullptr, &dockMainLeftRight);

			ImGui::DockBuilderDockWindow("Game View", dockMainLeftRight);
			ImGui::DockBuilderDockWindow("Level Outline", dockRightUp);
			ImGui::DockBuilderDockWindow("Object Property", dockRightDown);
			ImGui::DockBuilderDockWindow("Class Browser", dockMainLeftLeft);
			ImGui::DockBuilderDockWindow("Console", dockMainLeftDown);

			ImGui::DockBuilderFinish(editorDockspaceId);
		}
		// Should be put at last
		ImGui::DockSpace(editorDockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::End();
	}

	void EditorLayer::ShowGameView(bool* bShow)
	{
		if (ImGui::Begin("Game View", bShow))
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			glm::vec2 max = { window->InnerRect.Max.x, window->InnerRect.Max.y };
			glm::vec2 min = { window->InnerRect.Min.x, window->InnerRect.Min.y };
			glm::vec2 size = max - min;
			if (size != m_LastGameViewSize)
			{
				// Update camera aspect ratio when Game View window is resized
				OnGameViewWindowResized(size.x, size.y);
			}
			// Draw framebuffer texture
			ImGui::GetWindowDrawList()->AddImage(
				Renderer2D::GetStorageData()->MainFBO->GetRenderedTexture(),
				// Upper left corner for the UVs to be applied at
				window->InnerRect.Min,
				// Lower right corner for the UVs to be applied at
				window->InnerRect.Max,
				// The UVs have to be flipped
				ImVec2(0, 1), ImVec2(1, 0));
			max = { window->InnerRect.Max.x, window->InnerRect.Max.y };
			min = { window->InnerRect.Min.x, window->InnerRect.Min.y };
			m_LastGameViewSize = max - min;

			// Begin drop operation from Class Browser
			// Note: BeginDragDropTarget() does not support window as target
			if (ImGui::BeginDragDropTargetCustom(ImGui::GetCurrentWindow()->Rect(), ImGui::GetCurrentWindow()->ID))
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DragGameObjectClass"))
				{
					GameLayer* game = Application::Get().FindLayerByName<GameLayer>("Game");
					if (game)
					{
						// We use active camera instead of editor camera here because placing objects during PIE is allowed for now
						// It should be changed back to editor camera if that behavior is disabled
						const glm::vec2 result = ProjectScreenToWorld2D(glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y), ImGui::GetCurrentWindow(), game->GetActiveCamera());

						// Spawn dragged Game Object to the level at mouse position
						// Note: It sesems that rttr::argument does not support initializer_list conversion, so we should explicitly call constructor for glm::vec3 here
						rttr::variant createdVar = (*(rttr::type*)payload->Data).create({ glm::vec3{ result.x, result.y, 0.1f } });
						GameObject* spawnedGameObject = createdVar.get_value<GameObject*>();
						if (spawnedGameObject != m_SelectedGameObject)
						{
							OnGameObjectSelectionChanged(m_SelectedGameObject);
						}
						// Set it selected
						m_SelectedGameObject = spawnedGameObject;
						m_SelectedGameObject->m_bIsSelectedInEditor = true;
					}
				}
				ImGui::EndDragDropTarget();
			}

			// Transform options window
			if (g_PIEState == PIEState::None)
			{
				EditTransform();
			}

			//////////////////////////////////////////////////////////////////////////
			// ToolBar ///////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			
			// Place buttons at window center
			ImGui::Indent(ImGui::GetWindowSize().x / 2.0f - 40.0f);
			// Toggle play / stop
			if (ImGui::ImageButton(m_ToolBarTextures[0]->GetTexture(), ImVec2(32.0f, 32.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)))
			{
				if (g_PIEState == PIEState::None)
				{
					StartPIE();
				}
				else
				{
					StopPIE();
				}
			}
			ImGui::SameLine();
			// Toggle pause / resume
			if (ImGui::ImageButton(m_ToolBarTextures[1]->GetTexture(), ImVec2(32.0f, 32.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)))
			{
				if (g_PIEState == PIEState::Running)
				{
					PausePIE();
				}
				else if (g_PIEState == PIEState::Paused)
				{
					ResumePIE();
				}
			}

			if (m_SelectedGameObject && !m_SelectedGameObject->IsPendingDestroy())
			{
				m_SelectedGameObject->OnGameViewImGuiRender();
			}
			
			m_bIsHoveringGameView = ImGui::IsWindowHovered();
		}
		ImGui::End();
	}

	void EditorLayer::ShowLevelOutline(bool* bShow)
	{
		auto& level = Level::Get();

		if (ImGui::Begin("Level Outline", bShow))
		{
			ImGui::Text("(%d objects total)", level.m_SortedGameObjects.size());
			for (auto it = level.m_SortedGameObjects.begin(); it != level.m_SortedGameObjects.end(); ++it)
			{
				if (it->second->IsPendingDestroy())
					continue;

				ImVec4 color;
				if (it->second->IsActive())
				{
					// Translucent objects are marked yellow instead of white
					color = it->second->IsTranslucent() ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				else
				{
					// Inactive objects are marked darker
					color = it->second->IsTranslucent() ? ImVec4(0.75f, 0.75f, 0.0f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
				}

				// Clear other selections if a new GameObject is selected
				if (m_SelectedGameObject != it->second)
				{
					it->second->m_bIsSelectedInEditor = false;
				}
				std::stringstream ss;
				// Use pointer as unique id
				ss << "##" << it->second;
				// Note: Pass the non-visible label then we can use the space to draw colored text with ImGui::SameLine();
				if (ImGui::Selectable(ss.str().c_str(), &it->second->m_bIsSelectedInEditor))
				{
					// Select a new GameObject
					if (m_SelectedGameObject != it->second)
					{
						OnGameObjectSelectionChanged(m_SelectedGameObject);
						m_SelectedGameObject = it->second;
					}
				}
				// Begin dragging spawned GameObject from level outline
				// Target should be exposed GameObject* variables in Object Property window
				if (m_SelectedGameObject && ImGui::BeginDragDropSource())
				{
					// Note: The payload data of the drag operation should always be the variable's address, therefore, in order to pass a GameObject*, we need to pass it by GameObject**
					// see ProcessGameObjectType()
					ImGui::SetDragDropPayload("DragGameObject", &it->second, sizeof(GameObject**), ImGuiCond_Once);
					// Display object name as drag preview
					ImGui::Text("%s", it->second->GetName().c_str());
					ImGui::EndDragDropSource();
				}
				ImGui::SameLine();
				// Display GameObject's name
				ImGui::TextColored(color, "%s", it->second->GetName().c_str());
				// Show GameObject's unique name as tooltip
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Unique name: %s", it->second->GetUniqueName().c_str());
				}

				// Clear the variable if that GameObject is deselected
				if (m_SelectedGameObject == it->second && !it->second->m_bIsSelectedInEditor)
				{
					m_SelectedGameObject = nullptr;
				}
			}
			// Delete a GameObject
			// This part should be outside of that for loop as iterator will be invalidated
			if (m_SelectedGameObject && ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Delete)))
			{
				ZE_CORE_INFO("{0} deleted", m_SelectedGameObject->GetName());
				m_SelectedGameObject->Destroy();
				m_SelectedGameObject = nullptr;
			}
		}
		ImGui::End();
	}

	void EditorLayer::ShowObjectProperty(bool* bShow)
	{
		if (ImGui::Begin("Object Property", bShow))
		{
			if (m_SelectedGameObject && !m_SelectedGameObject->IsPendingDestroy())
			{
				if (m_bIsSortedPropertiesDirty)
				{
					PreProcessProperties();
				}
				m_bPropertyRecursed = false;
				ProcessPropertiesRecursively(m_SelectedGameObject);
			}
		}
		ImGui::End();
	}

	void EditorLayer::PreProcessProperties()
	{
		m_SortedProperties.clear();
		rttr::instance object = rttr::instance(m_SelectedGameObject);
		rttr::instance obj = object.get_type().get_raw_type().is_wrapper() ? object.get_wrapped_instance() : object;
		// Get the most derived class's properties
		auto& properties = obj.get_derived_type().get_properties();
		for (auto prop : properties)
		{
			// Properties with Hidden meta do not pass here
			if (prop.get_metadata(PropertyMeta::Hidden))
				continue;

			const rttr::variant& categoryVar = prop.get_metadata(PropertyMeta::Category);
			if (categoryVar)
			{
				const std::string& category = categoryVar.get_value<std::string>();
				m_SortedProperties[category].push_back(prop);
			}
			else
			{
				static const char* defaultCategory = "Default";
				m_SortedProperties[defaultCategory].push_back(prop);
			}
		}
		m_PropertiesLogged.reserve(properties.size());
		m_bIsSortedPropertiesDirty = false;
	}

	void EditorLayer::ProcessPropertiesRecursively(const rttr::instance& object)
	{
		rttr::instance obj = object.get_type().get_raw_type().is_wrapper() ? object.get_wrapped_instance() : object;
		// If it is the outermost property
		if (!m_bPropertyRecursed)
		{
			for (const auto& propPair : m_SortedProperties)
			{
				ImGui::Columns(1);
				// Display category seperator
				if (ImGui::CollapsingHeader(propPair.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					for (auto prop : propPair.second)
					{
						ProcessProperty(prop, obj);
					}
				}
			}
		}
		// Or it is a child property of a class/struct or possibly an element of a container
		else
		{
			if (obj.get_type().is_derived_from<GameObject>() && !m_bGameObjectPropWarningLogged)
			{
				m_bGameObjectPropWarningLogged = true;
				ZE_CORE_WARN("Failed registering property with type {0}! You should always register it with base type GameObject* and specify PropertyMeta::SubclassOf instead.", obj.get_type().get_name());
				return;
			}
			// Get the most derived class's properties
			auto& properties = obj.get_derived_type().get_properties();
			// Note: If custom struct/class is a pointer, it must not be null in order to retrieve its properties
			// @see GameObject::m_Transform, GameObject::m_CollisionData
			for (auto prop : properties)
			{
				ProcessProperty(prop, obj);
			}
		}
	}

	void EditorLayer::ProcessProperty(const rttr::property& prop, const rttr::instance& object)
	{
		rttr::variant& var = prop.get_value(object);
		if (!var)
			return;

		if (!ProcessPropertyValue(var, prop, object) && std::find(m_PropertiesLogged.begin(), m_PropertiesLogged.end(), prop) == m_PropertiesLogged.end())
		{
			m_PropertiesLogged.push_back(prop);
			ZE_CORE_ERROR("Failed to resolve value of property {0}!", prop.get_name());
		}
	}

	bool EditorLayer::ProcessPropertyValue(const rttr::variant& var, const rttr::property& prop, const rttr::instance& object)
	{
		rttr::type valueType = var.get_type();
		rttr::type wrappedType = valueType.is_wrapper() ? valueType.get_wrapped_type() : valueType;
		bool bIsWrapper = wrappedType != valueType;
		// Atomic types
		if (ProcessAtomicTypes(wrappedType, bIsWrapper ? var.extract_wrapped_value() : var, prop, object))
		{
		}
		// Sequential container types - this is not recognized as wrapper types
		else if (var.is_sequential_container())
		{
			if (wrappedType.is_pointer() && std::find(m_PropertiesLogged.begin(), m_PropertiesLogged.end(), prop) == m_PropertiesLogged.end())
			{
				m_PropertiesLogged.push_back(prop);
				ZE_CORE_WARN("Registering a pointer type sequential container {0} is not supported!", prop.get_name());
				return true;
			}
			rttr::variant_sequential_view sequentialView = var.create_sequential_view();
			if (sequentialView.get_rank() > 1 && std::find(m_PropertiesLogged.begin(), m_PropertiesLogged.end(), prop) == m_PropertiesLogged.end())
			{
				m_PropertiesLogged.push_back(prop);
				ZE_CORE_WARN("Registering a nested sequential container {0} is not supported!", prop.get_name());
				return true;
			}
			bool bOpen = ImGui::TreeNodeEx(prop.get_name().data());
			ShowPropertyTooltip(prop);
			if (bOpen)
			{
				// Switch to the right column
				ImGui::NextColumn();
				// Add insert and erase-all buttons
				AddSequentialButtons(prop, sequentialView);
				// Switch to next line's left column
				ImGui::NextColumn();
				ProcessSequentialContainerTypes(prop, sequentialView);
				prop.set_value(object, var);
				ImGui::TreePop();
			}
			else
			{
				// Switch to next line's left column
				ImGui::NextColumn();
				ImGui::NextColumn();
			}
		}
		// TODO: Associative container types
		else if (var.is_associative_container())
		{
			ZE_CORE_ASSERT_INFO(false, "Associative containers are currently not supported!");
			if (ImGui::TreeNodeEx(prop.get_name().data()))
			{
				auto associativeView = var.create_associative_view();
				ProcessAssociativeContainerTypes(associativeView, prop, object);
				prop.set_value(object, var);
				ImGui::TreePop();
			}
		}
		// class/struct types
		else
		{
			auto& childProps = bIsWrapper ? wrappedType.get_properties() : valueType.get_properties();
			if (!childProps.empty())
			{
				if (!wrappedType.is_pointer())
				{
					if (std::find(m_PropertiesLogged.begin(), m_PropertiesLogged.end(), prop) == m_PropertiesLogged.end())
					{
						m_PropertiesLogged.push_back(prop);
						ZE_CORE_WARN("Registering custom struct ({0}) by value is not supported! Try 'policy::prop::bind_as_ptr'.", prop.get_name());
					}
					return true;
				}
				ImGui::AlignTextToFramePadding();
				bool bOpen = ImGui::TreeNodeEx(prop.get_name().data(), ImGuiTreeNodeFlags_DefaultOpen);
				ShowPropertyTooltip(prop);
				// Switch to next line's left column
				ImGui::NextColumn();
				ImGui::NextColumn();
				if (bOpen)
				{
					m_bPropertyRecursed = true;
					ProcessPropertiesRecursively(var);
					// For transform specific type, we want to re-calculate transform matrix on value changes
					// TODO: This may need refactoring if Transform struct is replaced with component
					if (m_bIsTransformDirty && wrappedType.get_raw_type() == rttr::type::get<Transform>())
					{
						m_SelectedGameObject->RecomposeTransformMatrix();
						m_bIsTransformDirty = false;
					}
					ImGui::TreePop();
				}
			}
			else
			{
				return false;
			}
		}
		return true;
	}

	bool EditorLayer::ProcessAtomicTypes(const rttr::type& type, const rttr::variant& var, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		if (type.is_arithmetic())
		{
			if (type == rttr::type::get<bool>())
			{
				ProcessBoolType(var.to_bool(), prop, object, sequentialView, sequentialIndex);
			}
			else if (type == rttr::type::get<int8_t>())
			{
				ProcessInt8Type(var.to_int8(), prop, object, sequentialView, sequentialIndex);
			}
			else if (type == rttr::type::get<int32_t>())
			{
				ProcessInt32Type(var.to_int32(), prop, object, sequentialView, sequentialIndex);
			}
			else if (type == rttr::type::get<int64_t>())
			{
				ProcessInt64Type(var.to_int64(), prop, object, sequentialView, sequentialIndex);
			}
			else if (type == rttr::type::get<uint8_t>())
			{
				ProcessUInt8Type(var.to_uint8(), prop, object, sequentialView, sequentialIndex);
			}
			else if (type == rttr::type::get<uint32_t>())
			{
				ProcessUInt32Type(var.to_uint32(), prop, object, sequentialView, sequentialIndex);
			}
			else if (type == rttr::type::get<uint64_t>())
			{
				ProcessUInt64Type(var.to_uint64(), prop, object, sequentialView, sequentialIndex);
			}
			else if (type == rttr::type::get<float>())
			{
				ProcessFloatType(var.to_float(), prop, object, sequentialView, sequentialIndex);
			}
			else if (type == rttr::type::get<double>())
			{
				ProcessDoubleType(var.to_double(), prop, object, sequentialView, sequentialIndex);
			}
			return true;
		}
		// enum
		else if (type.is_enumeration())
		{
			ProcessEnumType(var, prop, object, sequentialView, sequentialIndex);
			return true;
		}
		// std::string
		else if (type.get_raw_type() == rttr::type::get<std::string>())
		{
			if (sequentialIndex == -1)
			{
				if (!type.is_pointer())
				{
					if (std::find(m_PropertiesLogged.begin(), m_PropertiesLogged.end(), prop) == m_PropertiesLogged.end())
					{
						m_PropertiesLogged.push_back(prop);
						ZE_CORE_WARN("Registering std::string ({0}) by value is not supported! Try 'policy::prop::bind_as_ptr'.", prop.get_name());
					}
				}
				else
				{
					ProcessStringType(var.get_value<std::string*>(), prop, sequentialView, sequentialIndex);
				}
			}
			else
			{
				ProcessStringType(const_cast<std::string*>(&var.get_value<std::string>()), prop, sequentialView, sequentialIndex);
			}
			return true;
		}
		// glm::vec2
		else if (type.get_raw_type() == rttr::type::get<glm::vec2>())
		{
			if (sequentialIndex == -1)
			{
				if (!type.is_pointer())
				{
					if (std::find(m_PropertiesLogged.begin(), m_PropertiesLogged.end(), prop) == m_PropertiesLogged.end())
					{
						m_PropertiesLogged.push_back(prop);
						ZE_CORE_WARN("Registering glm::vec2 ({0}) by value is not supported! Try 'policy::prop::bind_as_ptr'.", prop.get_name());
					}
				}
				else
				{
					ProcessVec2Type(var.get_value<glm::vec2*>(), prop, sequentialView, sequentialIndex);
				}
			}
			else
			{
				ProcessVec2Type(const_cast<glm::vec2*>(&var.get_value<glm::vec2>()), prop, sequentialView, sequentialIndex);
			}
			return true;
		}
		// glm::vec3
		else if (type.get_raw_type() == rttr::type::get<glm::vec3>())
		{
			if (sequentialIndex == -1)
			{
				if (!type.is_pointer())
				{
					if (std::find(m_PropertiesLogged.begin(), m_PropertiesLogged.end(), prop) == m_PropertiesLogged.end())
					{
						m_PropertiesLogged.push_back(prop);
						ZE_CORE_WARN("Registering glm::vec3 ({0}) by value is not supported! Try 'policy::prop::bind_as_ptr'.", prop.get_name());
					}
				}
				else
				{
					ProcessVec3Type(var.get_value<glm::vec3*>(), prop, sequentialView, sequentialIndex);
				}
			}
			else
			{
				ProcessVec3Type(const_cast<glm::vec3*>(&var.get_value<glm::vec3>()), prop, sequentialView, sequentialIndex);
			}
			return true;
		}
		// glm::vec4
		else if (type.get_raw_type() == rttr::type::get<glm::vec4>())
		{
			if (sequentialIndex == -1)
			{
				if (!type.is_pointer())
				{
					if (std::find(m_PropertiesLogged.begin(), m_PropertiesLogged.end(), prop) == m_PropertiesLogged.end())
					{
						m_PropertiesLogged.push_back(prop);
						ZE_CORE_WARN("Registering glm::vec4 ({0}) by value is not supported! Try 'policy::prop::bind_as_ptr'.", prop.get_name());
					}
				}
				else
				{
					ProcessColorType(var.get_value<glm::vec4*>(), prop, sequentialView, sequentialIndex);
				}
			}
			else
			{
				ProcessColorType(const_cast<glm::vec4*>(&var.get_value<glm::vec4>()), prop, sequentialView, sequentialIndex);
			}
			return true;
		}
		// GameObject*
		else if (type.is_pointer() && type == rttr::type::get<GameObject*>())
		{
			ProcessGameObjectType(var.get_value<GameObject*>(), prop, object, sequentialView, sequentialIndex);
			return true;                      
		}
		// Ref<Texture2D>
		else if (type.get_raw_type() == rttr::type::get<Texture2D>())
		{
			ProcessTexture2DType(var.get_value<Ref<Texture2D>>(), prop, object, sequentialView, sequentialIndex);
			return true;
		}

		return false;
	}

	void EditorLayer::AddSequentialButtons(const rttr::property& prop, rttr::variant_sequential_view& sequentialView)
	{
		rttr::type sequentialValueType = sequentialView.get_value_type();
		std::stringstream ss;
		ss << "##" << prop.get_name() << sequentialView.get_value_type().get_name();
		if (ImGui::BeginCombo(ss.str().c_str(), nullptr, ImGuiComboFlags_NoPreview))
		{
			ss.clear();
			ss.str("");
			ss << "Insert##" << prop.get_name() << sequentialView.get_value_type().get_name();
			// Insert a new item at the end
			if (ImGui::Selectable(ss.str().c_str()))
			{
				// TODO: We have to add every supported type explicitly for sequential container insertion, which is really not elegant
				if (sequentialValueType == rttr::type::get<bool>())
				{
					sequentialView.insert(sequentialView.end(), false);
				}
				else if (sequentialValueType == rttr::type::get<int8_t>())
				{
					sequentialView.insert(sequentialView.end(), (int8_t)0);
				}
				else if (sequentialValueType == rttr::type::get<int32_t>())
				{
					sequentialView.insert(sequentialView.end(), 0);
				}
				else if (sequentialValueType == rttr::type::get<int64_t>())
				{
					sequentialView.insert(sequentialView.end(), 0ll);
				}
				else if (sequentialValueType == rttr::type::get<uint8_t>())
				{
					sequentialView.insert(sequentialView.end(), (uint8_t)0);
				}
				else if (sequentialValueType == rttr::type::get<uint32_t>())
				{
					sequentialView.insert(sequentialView.end(), 0u);
				}
				else if (sequentialValueType == rttr::type::get<uint64_t>())
				{
					sequentialView.insert(sequentialView.end(), 0ull);
				}
				else if (sequentialValueType == rttr::type::get<float>())
				{
					sequentialView.insert(sequentialView.end(), 0.0f);
				}
				else if (sequentialValueType == rttr::type::get<double>())
				{
					sequentialView.insert(sequentialView.end(), 0.0);
				}
				else if (sequentialValueType.is_enumeration())
				{
					sequentialView.insert(sequentialView.end(), *sequentialValueType.get_enumeration().get_values().begin());
				}
				else if (sequentialValueType == rttr::type::get<std::string>())
				{
					sequentialView.insert(sequentialView.end(), std::string(""));
				}
				else if (sequentialValueType == rttr::type::get<glm::vec2>())
				{
					sequentialView.insert(sequentialView.end(), glm::vec2(0.0f));
				}
				else if (sequentialValueType == rttr::type::get<glm::vec3>())
				{
					sequentialView.insert(sequentialView.end(), glm::vec3(0.0f));
				}
				else if (sequentialValueType == rttr::type::get<glm::vec4>())
				{
					sequentialView.insert(sequentialView.end(), glm::vec4(0.0f));
				}
				else if (sequentialValueType == rttr::type::get<GameObject*>())
				{
					sequentialView.insert(sequentialView.end(), (GameObject*)nullptr);
				}
				else if (sequentialValueType == rttr::type::get<Ref<Texture2D>>())
				{
					sequentialView.insert(sequentialView.end(), Ref<Texture2D>());
				}
				else
				{
					ZE_CORE_WARN("Unknown sequential container type {0}!", prop.get_name());
				}
			}	
			if (!sequentialView.is_empty())
			{
				ss.clear();
				ss.str("");
				ss << "Erase all##" << prop.get_name() << sequentialView.get_value_type().get_name();
				// Clear all items
				if (ImGui::Selectable(ss.str().c_str()))
				{
					sequentialView.clear();
				}
			}
			ImGui::EndCombo();
		}
	}

	void EditorLayer::ProcessSequentialContainerTypes(const rttr::property& prop, rttr::variant_sequential_view& sequentialView)
	{
		for (uint32_t i = 0; i < sequentialView.get_size(); ++i)
		{
			rttr::variant& item = sequentialView.get_value(i);
			// TODO: Nested sequential containers not supported
			if (item.is_sequential_container())
			{
				std::stringstream ss;
				ss << "[" << i << "]";
				if (ImGui::TreeNodeEx(ss.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					sequentialView = item.create_sequential_view();
					ProcessSequentialContainerTypes(prop, sequentialView);
					ImGui::TreePop();
				}
			}
			else
			{
				rttr::variant& wrappedVar = item.extract_wrapped_value();
				rttr::type valueType = wrappedVar.get_type();
				rttr::type wrappedType = valueType.is_wrapper() ? valueType.get_wrapped_type() : valueType;
				bool bIsWrapper = wrappedType != valueType;
				if (ProcessAtomicTypes(wrappedType, bIsWrapper ? wrappedVar.extract_wrapped_value() : wrappedVar, prop, rttr::instance(), sequentialView, i))
				{
				}
				// TODO: Sequential containers of struct/class not supported for now because we cannot implement a strongly typed insertion function for now
				else
				{
					std::stringstream ss;
					ss << valueType.get_name() << "[" << i << "]";
					if (ImGui::TreeNodeEx(ss.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
					{
						m_bPropertyRecursed = true;
						ProcessPropertiesRecursively(wrappedVar);
						ImGui::TreePop();
					}
				}
			}
		}
	}

	void EditorLayer::ProcessAssociativeContainerTypes(const rttr::variant_associative_view& associativeView, const rttr::property& prop, const rttr::instance& object)
	{
		// i.e. std::set
		if (associativeView.is_key_only_type())
		{
			for (const auto& pair : associativeView)
			{
				ProcessPropertyValue(pair.first, prop, object);
			}
		}
		else
		{
			for (const auto& pair : associativeView)
			{
				ProcessPropertyValue(pair.first, prop, object);
				ImGui::SameLine();
				ProcessPropertyValue(pair.second, prop, object);
			}
		}
	}

	void EditorLayer::ShowClassBrowser(bool* bShow)
	{
		if (ImGui::Begin("Class Browser", bShow))
		{
			// Yellow hint text
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
			ImGui::TextWrapped("Drag a non-abstract GameObject class to Game View to place it to the level!");
			ImGui::PopStyleColor();

			m_DisplayedClasses.clear();
			// Display the root class - GameObject class
			if (ImGui::TreeNodeEx("GameObject (ABSTRACT)", ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (const auto& typePair : m_ClassInheritanceTree)
				{
					// Class iteration starts from GameObject type
					if (typePair.first.get_name() == "GameObject")
					{
						DisplayClassHierarchyRecursively(typePair.second);
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}

	void EditorLayer::ShowConsole(bool* bShow)
	{
		if (ImGui::Begin("Console", bShow))
		{

		}
		ImGui::End();
	}

	void EditorLayer::ShowParticleEditor(bool* bShow)
	{
		SetNextWindowDefaultPosition();
		ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Particle Editor", bShow))
		{

		}
		ImGui::End();
	}

	void EditorLayer::ShowPreferences(bool* bShow)
	{
		SetNextWindowDefaultPosition();
		ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Preferences", bShow, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::ShowStyleSelector("Editor style");
		}
		ImGui::End();
	}

	void EditorLayer::ShowAbout(bool* bShow)
	{
		SetNextWindowDefaultPosition();
		ImGui::SetNextWindowSize(ImVec2(300, 200));
		if (ImGui::Begin("About", bShow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
		{
			ImGui::TextCentered("ZeoEngine 0.1");
			ImGui::TextCentered("Created by SanSan");
			ImGui::TextCentered("https://github.com/hls333555/");
			static const float logoSize = 100.0f;
			// Center the logo
			ImGui::Indent((ImGui::GetWindowSize().x - logoSize) / 2.0f);
			ImGui::Image(m_LogoTexture->GetTexture(),
				ImVec2(logoSize, logoSize),
				ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}
		ImGui::End();
	}

	void EditorLayer::SetNextWindowDefaultPosition()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 100.0f, viewport->Pos.y + 100.0f), ImGuiCond_FirstUseEver);
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Start PIE by pressing Alt+P
		if ((Input::IsKeyPressed(ZE_KEY_LEFT_ALT) || Input::IsKeyPressed(ZE_KEY_RIGHT_ALT)) && Input::IsKeyPressed(ZE_KEY_P))
		{
			if (g_PIEState == PIEState::None)
			{
				StartPIE();
			}
		}
		// Exit PIE by pressing Esc
		if (Input::IsKeyPressed(ZE_KEY_ESCAPE))
		{
			if (g_PIEState != PIEState::None)
			{
				StopPIE();
			}
		}
		return false;
	}

	void EditorLayer::OnGameViewWindowResized(float newSizeX, float newSizeY)
	{
		// Update editor's camera
		m_EditorCameraController->UpdateProjection(newSizeX / newSizeY);

		GameLayer* game = Application::Get().FindLayerByName<GameLayer>("Game");
		if (game)
		{
			// Update game's camera
			game->GetGameCameraController()->UpdateProjection(newSizeX / newSizeY);
		}
	}

	void EditorLayer::OnGameObjectSelectionChanged(GameObject* lastSelectedGameObject)
	{
		m_bIsSortedPropertiesDirty = true;
		m_PropertiesLogged.clear();
		m_bGameObjectPropWarningLogged = false;
		m_TempInputStrings.clear();
	}

	void EditorLayer::EditTransform()
	{
		static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::TRANSLATE);
		static ImGuizmo::MODE currentGizmoMode(ImGuizmo::LOCAL);
		static bool bUseSnap = false;
		// We have to share snap values as Manipulate() only accepts one parameter for snapping
		static float snap[] = { 1.0f, 1.0f, 1.0f };
		static float localBounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
		static bool bUseBoundSizing = false;
		static bool bUseBoundSizingSnap = false;
		static float boundSizingSnap[] = { 0.1f, 0.1f, 0.1f };

		if (m_bIsHoveringGameView && ImGui::IsKeyPressed(ZE_KEY_W))
		{
			currentGizmoOperation = ImGuizmo::TRANSLATE;
		}
		if (m_bIsHoveringGameView && ImGui::IsKeyPressed(ZE_KEY_E))
		{
			currentGizmoOperation = ImGuizmo::ROTATE;
		}
		if (m_bIsHoveringGameView && ImGui::IsKeyPressed(ZE_KEY_R))
		{
			currentGizmoOperation = ImGuizmo::SCALE;
		}
		ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
		if (ImGui::Begin("Transform Options", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::RadioButton("Translate (W)", currentGizmoOperation == ImGuizmo::TRANSLATE))
			{
				currentGizmoOperation = ImGuizmo::TRANSLATE;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Rotate (E)", currentGizmoOperation == ImGuizmo::ROTATE))
			{
				currentGizmoOperation = ImGuizmo::ROTATE;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Scale (R)", currentGizmoOperation == ImGuizmo::SCALE))
			{
				currentGizmoOperation = ImGuizmo::SCALE;
			}
			if (currentGizmoOperation != ImGuizmo::SCALE)
			{
				if (ImGui::RadioButton("Local", currentGizmoMode == ImGuizmo::LOCAL))
				{
					currentGizmoMode = ImGuizmo::LOCAL;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("World", currentGizmoMode == ImGuizmo::WORLD))
				{
					currentGizmoMode = ImGuizmo::WORLD;
				}
			}

			// Note: ImGui::IsKeyPressed() will only fire once every frame if key is clicked
			// while Input::IsKeyPressed() will keep firing until key is released which is not good for toggle behaviors
			if (m_bIsHoveringGameView && ImGui::IsKeyPressed(ZE_KEY_S))
			{
				bUseSnap = !bUseSnap;
			}
			ImGui::Checkbox("##UseSnap", &bUseSnap);
			ImGui::SameLine();
			switch (currentGizmoOperation)
			{
			case ImGuizmo::TRANSLATE:
				ImGui::DragFloat3("Snap (S)", snap, 1.0f, 0.0f, 100.0f);
				break;
			case ImGuizmo::ROTATE:
				ImGui::DragFloat("Angle Snap (S)", snap, 1.0f, 0.0f, 120.0f);
				break;
			case ImGuizmo::SCALE:
				ImGui::DragFloat("Scale Snap (S)", snap, 1.0f, 0.0f, 10.0f);
				break;
			}
			ImGui::Checkbox("Bound Sizing", &bUseBoundSizing);
			if (bUseBoundSizing)
			{
				ImGui::Checkbox("##UseBoundSizingSnap", &bUseBoundSizingSnap);
				ImGui::SameLine();
				ImGui::InputFloat3("Bound Sizing Snap", boundSizingSnap);
			}
		}
		ImGui::End();

		// Draw transform gizmo on the selected GameObject when not in PIE
		// as editor camera, which gizmo needs, will get deactivated in PIE
		if (m_SelectedGameObject && !m_SelectedGameObject->IsPendingDestroy())
		{
			ImGuizmo::SetDrawlist();
			// TODO: Not applied to 3D rendering
			ImGuizmo::SetOrthographic(true);

			ImGuiWindow* window = ImGui::GetCurrentWindow();
			ImGuizmo::SetRect(window->InnerRect.Min.x, window->InnerRect.Min.y, window->InnerRect.GetSize().x, window->InnerRect.GetSize().y);

			ImGuizmo::Manipulate(glm::value_ptr(GetEditorCamera()->GetViewMatrix()), glm::value_ptr(GetEditorCamera()->GetProjectionMatrix()),
				currentGizmoOperation, currentGizmoMode,
				glm::value_ptr(m_SelectedGameObject->m_TransformMatrix), nullptr,
				bUseSnap ? &snap[0] : nullptr,
				bUseBoundSizing ? localBounds : nullptr,
				bUseBoundSizingSnap ? boundSizingSnap : nullptr);
			m_SelectedGameObject->DecomposeTransformMatrix();
		}
	}

	const char* PIETempFile("temp/PIE.tmp");

	void EditorLayer::StartPIE()
	{
		// Save level to a temp file
		Level::Get().SaveLevelToFile(std::string(PIETempFile), true);

		m_ToolBarTextures[0] = m_StopTexture;
		g_PIEState = PIEState::Running;
		ZE_CORE_TRACE("PIE started");
	}

	void EditorLayer::StopPIE()
	{
		// Load saved temp file
		Level::Get().LoadLevelFromFile(PIETempFile, true);
		// Clear cache
		std::fstream(PIETempFile, std::ios::out);

		m_ToolBarTextures[0] = m_PlayTexture;
		m_ToolBarTextures[1] = m_PauseTexture;
		g_PIEState = PIEState::None;
		ZE_CORE_TRACE("PIE stopped");
	}

	void EditorLayer::PausePIE()
	{
		m_ToolBarTextures[1] = m_PlayTexture;
		g_PIEState = PIEState::Paused;
		ZE_CORE_TRACE("PIE paused");
	}

	void EditorLayer::ResumePIE()
	{
		m_ToolBarTextures[1] = m_PauseTexture;
		g_PIEState = PIEState::Running;
		ZE_CORE_TRACE("PIE resumed");
	}

	void EditorLayer::ConstructClassInheritanceTree()
	{
		m_ClassInheritanceTree.clear();
		rttr::type baseType = rttr::type::get_by_name("GameObject");
		auto& derivedTypes = baseType.get_derived_classes();
		ConstructClassInheritanceTreeRecursively(baseType, derivedTypes);
	}

	void EditorLayer::ConstructClassInheritanceTreeRecursively(const rttr::type& baseType, const rttr::array_range<rttr::type>& derivedTypes)
	{
		for (const auto type : derivedTypes)
		{
			auto& dTypes = type.get_derived_classes();
			// We assume the last element in this class's base classes array is this class's (direct) parent class
			// because we only want to store direct parent-children relations in one key-value pair
			if (*type.get_base_classes().rbegin() == baseType)
			{
				m_ClassInheritanceTree[baseType].push_back(type);
			}
			if (!dTypes.empty())
			{
				ConstructClassInheritanceTreeRecursively(type, dTypes);
			}
		}
	}

	void EditorLayer::DisplayClassHierarchyRecursively(const std::vector<rttr::type>& derivedTypes)
	{
		for (const auto type : derivedTypes)
		{
			// If we find this type in the map - current class has child classes
			if (m_ClassInheritanceTree.find(type) != m_ClassInheritanceTree.end())
			{
				// If this class type is not handled yet
				if (m_DisplayedClasses.find(type) == m_DisplayedClasses.end())
				{
					std::stringstream ss;
					if (type.get_metadata(ClassMeta::Abstract))
					{
						ss << type.get_name() << " (ABSTRACT)";
					}
					else
					{
						ss << type.get_name();
					}
					// Display it as a parent node
					bool bOpen = ImGui::TreeNodeEx(ss.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen);
					// Mark it handled
					m_DisplayedClasses.insert(type);
					ProcessClassInteraction(type);
					if (bOpen)
					{
						// Iteratively display its child classes if this node is opened
						DisplayClassHierarchyRecursively(m_ClassInheritanceTree[type]);
						ImGui::TreePop();
					}
					else
					{
						// Even if this node is closed, we do not want to display its child classes later as they are hidden inside this node
						auto& derivedTypes = type.get_derived_classes();
						for (const auto& dType : derivedTypes)
						{
							m_DisplayedClasses.insert(dType);
						}
					}
				}
			}
			// If this type is not found - current class does not have child classes
			else
			{
				// If this class type is not handled yet
				if (m_DisplayedClasses.find(type) == m_DisplayedClasses.end())
				{
					std::stringstream ss;
					if (type.get_metadata(ClassMeta::Abstract))
					{
						ss << type.get_name() << " (ABSTRACT)";
					}
					else
					{
						ss << type.get_name();
					}
					// Display it as a bullet child node
					ImGui::TreeNodeEx(ss.str().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
					// Mark it handled
					m_DisplayedClasses.insert(type);
					ProcessClassInteraction(type);
				}
			}
		}
	}

	void EditorLayer::ProcessClassInteraction(const rttr::type& type)
	{
		ShowTypeTooltip(type);
		// Abstract classes cannot get spawned to the level
		if (!type.get_metadata(ClassMeta::Abstract))
		{
			// Begin dragging a GameObject class from Class Browser
			// Target should be Game View window
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("DragGameObjectClass", &type, sizeof(rttr::type), ImGuiCond_Once);
				// Display class name as drag preview
				ImGui::Text("%s", type.get_name().data());
				ImGui::EndDragDropSource();
			}
		}
	}

	void EditorLayer::ShowTypeTooltip(const rttr::type& type)
	{
		if (ImGui::IsItemHovered())
		{
			rttr::variant tooltipVar = type.get_metadata(ClassMeta::Tooltip);
			if (tooltipVar)
			{
				ImGui::SetTooltip(tooltipVar.get_value<std::string>().c_str());
			}
		}
	}

	void EditorLayer::ShowPropertyTooltip(const rttr::property& prop)
	{
		if (ImGui::IsItemHovered())
		{
			rttr::variant tooltipVar = prop.get_metadata(PropertyMeta::Tooltip);
			if (tooltipVar)
			{
				ImGui::SetTooltip(tooltipVar.get_value<std::string>().c_str());
			}
		}
	}

	void EditorLayer::ProcessBoolType(bool boolValue, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		ImGui::Checkbox(ss.str().c_str(), &boolValue);
		if (sequentialIndex == -1)
		{
			END_SETPROP(prop, object, boolValue)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, boolValue, false)
		}
	}

	void EditorLayer::ProcessInt8Type(int8_t int8Value, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		rttr::variant minVar = prop.get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = prop.get_metadata(PropertyMeta::Max);
		rttr::variant speedVar = prop.get_metadata(PropertyMeta::DragSensitivity);
		int8_t min = minVar ? std::max(minVar.to_int8(), (int8_t)INT8_MIN) : (int8_t)INT8_MIN;
		int8_t max = maxVar ? std::min(maxVar.to_int8(), (int8_t)INT8_MAX) : (int8_t)INT8_MAX;
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		ImGui::DragInt_8(ss.str().c_str(), &int8Value, speed, min, max);
		if (sequentialIndex == -1)
		{
			END_SETPROP(prop, object, int8Value)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, int8Value, (int8_t)0)
		}
	}

	void EditorLayer::ProcessInt32Type(int32_t int32Value, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		rttr::variant minVar = prop.get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = prop.get_metadata(PropertyMeta::Max);
		rttr::variant speedVar = prop.get_metadata(PropertyMeta::DragSensitivity);
		int32_t min = minVar ? std::max(minVar.to_int32(), INT32_MIN) : INT32_MIN;
		int32_t max = maxVar ? std::min(maxVar.to_int32(), INT32_MAX) : INT32_MAX;
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		ImGui::DragInt(ss.str().c_str(), &int32Value, speed, min, max);
		if (sequentialIndex == -1)
		{
			END_SETPROP(prop, object, int32Value)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, int32Value, 0)
		}
	}

	void EditorLayer::ProcessInt64Type(int64_t int64Value, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		rttr::variant minVar = prop.get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = prop.get_metadata(PropertyMeta::Max);
		rttr::variant speedVar = prop.get_metadata(PropertyMeta::DragSensitivity);
		int64_t min = minVar ? std::max(minVar.to_int64(), INT64_MIN) : INT64_MIN;
		int64_t max = maxVar ? std::min(maxVar.to_int64(), INT64_MAX) : INT64_MAX;
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		ImGui::DragInt_64(ss.str().c_str(), &int64Value, speed, min, max);
		if (sequentialIndex == -1)
		{
			END_SETPROP(prop, object, int64Value)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, int64Value, 0ll)
		}
	}

	void EditorLayer::ProcessUInt8Type(uint8_t uint8Value, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		rttr::variant minVar = prop.get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = prop.get_metadata(PropertyMeta::Max);
		rttr::variant speedVar = prop.get_metadata(PropertyMeta::DragSensitivity);
		uint8_t min = minVar ? std::max(minVar.to_uint8(), (uint8_t)0) : 0;
		uint8_t max = maxVar ? std::min(maxVar.to_uint8(), (uint8_t)UINT8_MAX) : (uint8_t)UINT8_MAX;
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		ImGui::DragUInt_8(ss.str().c_str(), &uint8Value, speed, min, max);
		if (sequentialIndex == -1)
		{
			END_SETPROP(prop, object, uint8Value)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, uint8Value, (uint8_t)0)
		}
	}

	void EditorLayer::ProcessUInt32Type(uint32_t uint32Value, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		rttr::variant minVar = prop.get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = prop.get_metadata(PropertyMeta::Max);
		rttr::variant speedVar = prop.get_metadata(PropertyMeta::DragSensitivity);
		uint32_t min = minVar ? std::max(minVar.to_uint32(), (uint32_t)0) : 0;
		uint32_t max = maxVar ? std::min(maxVar.to_uint32(), (uint32_t)UINT32_MAX) : (uint32_t)UINT32_MAX;
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		ImGui::DragUInt_32(ss.str().c_str(), &uint32Value, speed, min, max);
		if (sequentialIndex == -1)
		{
			END_SETPROP(prop, object, uint32Value)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, uint32Value, 0u)
		}
	}

	void EditorLayer::ProcessUInt64Type(uint64_t uint64Value, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		rttr::variant minVar = prop.get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = prop.get_metadata(PropertyMeta::Max);
		rttr::variant speedVar = prop.get_metadata(PropertyMeta::DragSensitivity);
		uint64_t min = minVar ? std::max(minVar.to_uint64(), (uint64_t)0) : 0;
		uint64_t max = maxVar ? std::min(maxVar.to_uint64(), (uint64_t)UINT64_MAX) : (uint64_t)UINT64_MAX;
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		ImGui::DragUInt_64(ss.str().c_str(), &uint64Value, speed, min, max);
		if (sequentialIndex == -1)
		{
			END_SETPROP(prop, object, uint64Value)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, uint64Value, 0ull)
		}
	}

	void EditorLayer::ProcessFloatType(float floatValue, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		rttr::variant minVar = prop.get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = prop.get_metadata(PropertyMeta::Max);
		rttr::variant speedVar = prop.get_metadata(PropertyMeta::DragSensitivity);
		// Note: The min value of float should be -FLT_MAX
		float min = minVar ? std::max(minVar.to_float(), -FLT_MAX) : -FLT_MAX;
		float max = maxVar ? std::min(maxVar.to_float(), FLT_MAX) : FLT_MAX;
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		// If transform's property value changes, mark it dirty (2D rotation only)
		if (ImGui::DragFloat(ss.str().c_str(), &floatValue, speed, min, max, "%.2f") && prop.get_declaring_type().get_raw_type() == rttr::type::get<Transform>())
		{
			m_bIsTransformDirty = true;
		}
		if (sequentialIndex == -1)
		{
			END_SETPROP(prop, object, floatValue)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, floatValue, 0.0f)
		}
	}

	void EditorLayer::ProcessDoubleType(double doubleValue, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		rttr::variant minVar = prop.get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = prop.get_metadata(PropertyMeta::Max);
		rttr::variant speedVar = prop.get_metadata(PropertyMeta::DragSensitivity);
		// Note: The min value of double should be -DBL_MAX
		double min = minVar ? std::max(minVar.to_double(), -DBL_MAX) : -DBL_MAX;
		double max = maxVar ? std::min(maxVar.to_double(), DBL_MAX) : DBL_MAX;
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		ImGui::DragDouble(ss.str().c_str(), &doubleValue, speed, min, max);
		if (sequentialIndex == -1)
		{
			END_SETPROP(prop, object, doubleValue)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, doubleValue, 0.0)
		}
	}

	void EditorLayer::ProcessEnumType(const rttr::variant& var, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		bool bOK = false;
		std::string stringValue = var.to_string(&bOK);
		if (!bOK && std::find(m_PropertiesLogged.begin(), m_PropertiesLogged.end(), prop) == m_PropertiesLogged.end())
		{
			m_PropertiesLogged.push_back(prop);
			ZE_CORE_WARN("Failed to convert an enum value of property {0} to string!", prop.get_name());
		}
		rttr::enumeration enumType = var.get_type().get_enumeration();
		auto& enumValues = enumType.get_values();
		if (ImGui::BeginCombo(ss.str().c_str(), stringValue.c_str()))
		{
			for (const auto& enumValue : enumValues)
			{
				if (ImGui::Selectable(enumType.value_to_name(enumValue).data()))
				{
					if (sequentialIndex == -1)
					{
						prop.set_value(object, enumValue);
					}
					else
					{
						sequentialView.set_value(sequentialIndex, enumValue);
					}
					// Selection changes, call the callback
					if (enumValue != enumType.name_to_value(stringValue))
					{
						object.get_type().invoke("OnPropertyValueChange", object, { prop });
					}
				}
			}
			ImGui::EndCombo();
		}
		if (sequentialIndex != -1)
		{
			ADD_SEQBUTTONS(prop, sequentialView, sequentialIndex, *enumValues.begin())
		}
		END_PROP(prop)
	}

	void EditorLayer::ProcessStringType(std::string* stringPointerValue, const rttr::property& prop, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		ImGui::InputText(ss.str().c_str(),
			m_TempInputStrings[ss.str().c_str()].first ?
			&m_TempInputStrings[ss.str().c_str()].second :
			stringPointerValue,
			ImGuiInputTextFlags_AutoSelectAll);
		// Write changes to cache first
		if (ImGui::IsItemActivated())
		{
			m_TempInputStrings[ss.str().c_str()].first = true;
			m_TempInputStrings[ss.str().c_str()].second = *stringPointerValue;
		}
		// Apply cache when user finishes editing
		if (ImGui::IsItemDeactivated())
		{
			if (prop.get_name() == "Name")
			{
				Level::Get().m_ObjectNames.erase(*stringPointerValue);
				Level::Get().m_ObjectNames.insert(m_TempInputStrings[ss.str().c_str()].second);
			}
			*stringPointerValue = m_TempInputStrings[ss.str().c_str()].second;
			m_TempInputStrings[ss.str().c_str()].first = false;
			m_TempInputStrings[ss.str().c_str()].second.clear();
		}
		if (sequentialIndex == -1)
		{
			END_PROP(prop)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, *stringPointerValue, std::string(""));
		}
	}

	void EditorLayer::ProcessVec2Type(glm::vec2* vec2PointerValue, const rttr::property& prop, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		rttr::variant minVar = prop.get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = prop.get_metadata(PropertyMeta::Max);
		rttr::variant speedVar = prop.get_metadata(PropertyMeta::DragSensitivity);
		float min = minVar ? std::max(minVar.to_float(), -FLT_MAX) : -FLT_MAX;
		float max = maxVar ? std::min(maxVar.to_float(), FLT_MAX) : FLT_MAX;
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		// If transform's property value changes, mark it dirty (2D scale only)
		if (ImGui::DragFloat2(ss.str().c_str(), glm::value_ptr(*vec2PointerValue), speed, min, max, "%.2f") && prop.get_declaring_type().get_raw_type() == rttr::type::get<Transform>())
		{
			m_bIsTransformDirty = true;
		}
		if (sequentialIndex == -1)
		{
			END_PROP(prop)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, *vec2PointerValue, glm::vec2(0.0f));
		}
	}

	void EditorLayer::ProcessVec3Type(glm::vec3* vec3PointerValue, const rttr::property& prop, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		rttr::variant minVar = prop.get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = prop.get_metadata(PropertyMeta::Max);
		rttr::variant speedVar = prop.get_metadata(PropertyMeta::DragSensitivity);
		float min = minVar ? std::max(minVar.to_float(), -FLT_MAX) : -FLT_MAX;
		float max = maxVar ? std::min(maxVar.to_float(), FLT_MAX) : FLT_MAX;
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		bool bIsTransformProp = prop.get_declaring_type().get_raw_type() == rttr::type::get<Transform>();
		// If transform's property value changes, mark it dirty
		if (ImGui::DragFloat3(ss.str().c_str(), glm::value_ptr(*vec3PointerValue), speed, min, max, "%.2f") && bIsTransformProp)
		{
			m_bIsTransformDirty = true;
		}
		// If position value changes, schedule to re-sort translucent objects
		// TODO: Not optimal for 3D rendering as modifying rotation and scale will invoke this too
		if (ImGui::IsItemDeactivatedAfterEdit() && bIsTransformProp)
		{
			Level::Get().OnTranslucentObjectsDirty(m_SelectedGameObject);
		}
		if (sequentialIndex == -1)
		{
			END_PROP(prop)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, *vec3PointerValue, glm::vec3(0.0f));
		}
	}

	void EditorLayer::ProcessColorType(glm::vec4* vec4PointerValue, const rttr::property& prop, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		ImGui::ColorEdit4(ss.str().c_str(), glm::value_ptr(*vec4PointerValue));
		if (sequentialIndex == -1)
		{
			END_PROP(prop)
		}
		else
		{
			END_SETSEQPROP(prop, sequentialView, sequentialIndex, *vec4PointerValue, glm::vec4(0.0f));
		}
	}

	void EditorLayer::ProcessGameObjectType(GameObject* gameObjectValue, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		std::stringstream ss_PreviewName;
		if (gameObjectValue && !gameObjectValue->IsPendingDestroy())
		{
			// Displayed as "ObjectName (ClassName)"
			ss_PreviewName << gameObjectValue->GetName() << " (" << rttr::type::get(*gameObjectValue).get_name() << ")";
		}
		// If current chosen GameObject is deleted from level, zero the pointer
		else
		{
			if (sequentialIndex == -1)
			{
				prop.set_value(object, nullptr);
			}
			else
			{
				sequentialView.set_value(sequentialIndex, (GameObject*)nullptr);
			}
		}
		if (ImGui::BeginCombo(ss.str().c_str(), ss_PreviewName.str().c_str()))
		{
			// A specialized entry for clearing out current selection (empty the pointer)
			if (ImGui::Selectable("Null"))
			{
				if (sequentialIndex == -1)
				{
					prop.set_value(object, nullptr);
				}
				else
				{
					sequentialView.set_value(sequentialIndex, (GameObject*)nullptr);
				}
			}

			auto& level = Level::Get();
			auto gameObjectCount = level.m_GameObjects.size();
			for (uint32_t i = 0; i < gameObjectCount; ++i)
			{
				if (!IsSubclassOf(level.m_GameObjects[i], prop))
					continue;

				std::stringstream ss_ObjectName;
				ss_ObjectName << level.m_GameObjects[i]->GetName() << " (" << rttr::type::get(*level.m_GameObjects[i]).get_name() << ")";
				if (ImGui::Selectable(ss_ObjectName.str().c_str()))
				{
					// Note: The pointer assignment will only succeed if assigned-from pointer type is exactly the same as assigned-to pointer type
					// That's to say, GameObject* cannot be assigned to Player* even if that GameObject* object is indeed a Player* object
					if (sequentialIndex == -1)
					{
						prop.set_value(object, level.m_GameObjects[i]);
					}
					else
					{
						sequentialView.set_value(sequentialIndex, level.m_GameObjects[i]);
					}
				}
			}
			ImGui::EndCombo();
		}
		// Allow assigning a spawned GameObject from level outline
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DragGameObject"))
			{
				// see ShowLevelOutline()
				GameObject* draggedGameObject = *(GameObject**)payload->Data;
				if (IsSubclassOf(draggedGameObject, prop))
				{
					if (sequentialIndex == -1)
					{
						prop.set_value(object, draggedGameObject);
					}
					else
					{
						sequentialView.set_value(sequentialIndex, draggedGameObject);
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (sequentialIndex != -1)
		{
			ADD_SEQBUTTONS(prop, sequentialView, sequentialIndex, (GameObject*)nullptr)
		}
		END_PROP(prop)
	}

	void EditorLayer::ProcessTexture2DType(const Ref<Texture2D>& texture2DValue, const rttr::property& prop, const rttr::instance& object, rttr::variant_sequential_view& sequentialView, int32_t sequentialIndex)
	{
		std::stringstream ss;
		if (sequentialIndex == -1)
		{
			BEGIN_PROP(prop)
		}
		else
		{
			BEGIN_SEQPROP(prop, sequentialIndex)
		}
		Texture2DLibrary* library = GetTexture2DLibrary();
		// TODO: Add an right-click option to draw texture smaller
		// Try to align texture'a width to column's right side
		float textureWidth = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - ImGui::GetCursorScreenPos().x - 18.5f;
		Ref<Texture2D> backgroundTexture = library->Get("editor_assets/textures/Checkerboard_Alpha.png");
		// Draw checkerboard texture as background first
		ImGui::GetWindowDrawList()->AddImage(backgroundTexture->GetTexture(),
			ImGui::GetCursorScreenPos(),
			ImVec2(ImGui::GetCursorScreenPos().x + textureWidth, ImGui::GetCursorScreenPos().y + textureWidth),
			ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		// Draw our texture on top of that
		ImGui::Image(texture2DValue ? texture2DValue->GetTexture() : nullptr,
			ImVec2(textureWidth, textureWidth),
			ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f),
			texture2DValue ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.0f), ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		// Display texture info tooltip
		if (texture2DValue && ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Resolution: %dx%d\nHas alpha: %s", texture2DValue->GetWidth(), texture2DValue->GetHeight(), texture2DValue->HasAlpha() ? "true" : "false");
		}
		if (sequentialIndex == -1)
		{
			// Align combo box's width to column's right side
			ImGui::SetNextItemWidth(-1.0f);
		}
		else
		{
			// Make room for sequential buttons
			ImGui::SetNextItemWidth(-25.0f);
		}
		// TODO: Add search filter for texture2D
		if (ImGui::BeginCombo(ss.str().c_str(), texture2DValue ? texture2DValue->GetFileName().c_str() : nullptr))
		{
			// Pop up file browser to select a texture
			if (ImGui::Selectable("Browse texture..."))
			{
				nfdchar_t* outPath = nullptr;
				nfdresult_t result = NFD_OpenDialog("png", nullptr, &outPath);
				if (result == NFD_OKAY)
				{
					const std::string relativePath = FormatPath(outPath);
					Ref<Texture2D> loadedTexture;
					// Add selected texture to the library
					if (library->Exists(relativePath))
					{
						loadedTexture = library->Get(relativePath);
					}
					else
					{
						loadedTexture = library->Load(relativePath);
					}
					if (sequentialIndex == -1)
					{
						prop.set_value(object, loadedTexture);
					}
					else
					{
						sequentialView.set_value(sequentialIndex, loadedTexture);
					}
					free(outPath);
				}
				else if (result == NFD_ERROR)
				{
					ZE_CORE_ERROR("ProcessTexture2DType: {0}", NFD_GetError());
				}
			}
			ImGui::Separator();
			// List all loaded textures from Texture2DLibrary
			for (const auto& [path, texture] : library->GetTexturesMap())
			{
				if (ImGui::Selectable(texture->GetFileName().c_str()))
				{
					if (sequentialIndex == -1)
					{
						prop.set_value(object, texture);
					}
					else
					{
						sequentialView.set_value(sequentialIndex, texture);
					}
				}
				// Display texture path tooltip
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", texture->GetPath().c_str());
				}
			}

			ImGui::EndCombo();
		}
		// Display texture path tooltip
		if (texture2DValue && ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", texture2DValue->GetPath().c_str());
		}
		if (sequentialIndex != -1)
		{
			ADD_SEQBUTTONS(prop, sequentialView, sequentialIndex, Ref<Texture2D>())
		}
		END_PROP(prop)
	}

	bool EditorLayer::IsSubclassOf(GameObject* gameObject, const rttr::property& prop)
	{
		// Note: rttr::type::get() should take an object instance instead of pointer as its parameter to get the correct object type!
		// So here, we filter out GameObjects not derived from this specified class
		// We retrieve the name of the filtering class from this property's metadata
		rttr::type objectType = rttr::type::get(*gameObject);
		rttr::variant subclassVar = prop.get_metadata(PropertyMeta::SubclassOf);
		if (subclassVar)
		{
			rttr::type baseType = rttr::type::get_by_name(subclassVar.get_value<std::string>());
			if (objectType.is_derived_from(baseType))
			{
				return true;
			}
		}
		else
		{
			// Still return true because the user does not specify PropertyMeta::SubclassOf
			return true;
		}
		return false;
	}

	std::string EditorLayer::FormatPath(const char* absolutePath)
	{
		std::filesystem::path path(absolutePath);
		// Convert abosulte path to relative path
		path = std::filesystem::relative(path, std::filesystem::current_path());
		std::string str = path.string();
		// Replace all '\\' with '/'
		for (uint32_t i = 0; i < str.size(); ++i)
		{
			if (str[i] == '\\')
			{
				str.replace(i, 1, "/");
			}
		}
		return str;
	}

}
