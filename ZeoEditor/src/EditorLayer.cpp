#include "EditorLayer.h"

#include <filesystem>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo.h>
#include <nfd.h>

#include "Engine/Core/Application.h"
#include "Engine/Core/EngineGlobals.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/EngineUtilities.h"
#include "Engine/Debug/BenchmarkTimer.h"
#include "Engine/Core/Serializer.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/GameFramework/Components.h"
#include "Dockspaces/MainDockspace.h"
#include "Dockspaces/ParticleEditorDockspace.h"

namespace ZeoEngine {

#define SHOW_IMGUI_DEMO 0

	PIEState pieState;

	EditorLayer::EditorLayer()
		: EngineLayer("Editor")
	{
	}

	void EditorLayer::OnAttach()
	{
		EngineLayer::OnAttach();

		Level& level = Level::Get();
		level.Init();
		level.m_OnLevelCleanUp += GET_MEMBER_FUNC(this, &EditorLayer::ClearSelectedGameObject);

		MainDockspace* mainDockspace = new MainDockspace("Zeo Editor", true, { 5.0f, 5.0f }, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
		PushDockspace(mainDockspace);

		std::string cachePath = std::filesystem::current_path().string().append("/temp");
		// Create temp folder on demand
		std::filesystem::create_directory(cachePath);
	}

	void EditorLayer::OnDetach()
	{
		EngineLayer::OnDetach();
	}

	void EditorLayer::OnUpdate(DeltaTime dt)
	{
		EngineLayer::OnUpdate(dt);

		m_DockspaceManager.OnUpdate(dt);

		//if (m_bShowParticleEditor)
		//{
		//	m_ParticleViewportPanel.OnUpdate(dt);

		//	if (m_EditorParticleSystem)
		//	{
		//		bool bAutoDestroy = m_EditorParticleSystem->GetAutoDestroy();
		//		m_EditorParticleSystem->SetParticleEditorPreviewMode(true, false);
		//		m_EditorParticleSystem->OnUpdate(dt);
		//		m_EditorParticleSystem->SetParticleEditorPreviewMode(false, bAutoDestroy);
		//	}

		//}
	}

	void EditorLayer::OnImGuiRender()
	{
#if SHOW_IMGUI_DEMO
		static bool bShow = false;
		ImGui::ShowDemoWindow(&bShow);
#endif

		// TODO: These bools are not saved out, so last opened windows cannot restore
		static bool bShowGameView = true;
		static bool bShowSceneOutline = true;
		static bool bShowObjectInspector = true;
		static bool bShowClassBrowser = true;

		//////////////////////////////////////////////////////////////////////////
		// EditorDockspace ///////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		m_DockspaceManager.OnImGuiRender();

		//////////////////////////////////////////////////////////////////////////
		// MainMenuBar ///////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		// TODO: Key shortcuts

		//if (ImGui::BeginMainMenuBar())
		//{
		//	// TODO: Test these if we have another window containing these named menus

		//	// File menu
		//	if (ImGui::BeginMenu("File"))
		//	{
		//		static bool bEnableLoadingOrSaving = true;
		//		// Loading and saving level during PIE is not allowed
		//		if (pieState != PIEState::None)
		//		{
		//			bEnableLoadingOrSaving = false;
		//		}
		//		else
		//		{
		//			bEnableLoadingOrSaving = true;
		//		}
		//		if (ImGui::MenuItem("New level", "CTRL+N", false, bEnableLoadingOrSaving))
		//		{
		//			Level::Get().CleanUp();
		//			m_CurrentLevelPath.clear();
		//			m_CurrentLevelName.clear();
		//		}
		//		if (ImGui::MenuItem("Open level", "CTRL+O", false, bEnableLoadingOrSaving))
		//		{
		//			nfdchar_t* outPath = nullptr;
		//			nfdresult_t result = NFD_OpenDialog("zlevel", nullptr, &outPath);
		//			if (result == NFD_OKAY)
		//			{
		//				Level::Get().LoadLevelFromFile(outPath);
		//				m_CurrentLevelPath = outPath;
		//				m_CurrentLevelName = std::filesystem::path(outPath).filename().string();
		//				free(outPath);
		//			}
		//			else if (result == NFD_ERROR)
		//			{
		//				ZE_CORE_ERROR("Open level failed: {0}", NFD_GetError());
		//			}
		//		}
		//		if (ImGui::MenuItem("Save level", "CTRL+S", false, bEnableLoadingOrSaving))
		//		{
		//			if (m_CurrentLevelPath.empty())
		//			{
		//				nfdchar_t* outPath = nullptr;
		//				nfdresult_t result = NFD_SaveDialog("zlevel", nullptr, &outPath);
		//				if (result == NFD_OKAY)
		//				{
		//					std::string pathStr = outPath;
		//					free(outPath);
		//					Level::Get().SaveLevelToFile(pathStr);
		//					m_CurrentLevelPath = pathStr;
		//					static const char* levelFileSuffix = ".zlevel";
		//					if (pathStr.rfind(levelFileSuffix) == std::string::npos)
		//					{
		//						pathStr += levelFileSuffix;
		//					}
		//					m_CurrentLevelName = std::filesystem::path(pathStr).filename().string();
		//				}
		//				else if (result == NFD_ERROR)
		//				{
		//					ZE_CORE_ERROR("Save level failed: {0}", NFD_GetError());
		//				}
		//			}
		//			else
		//			{
		//				Level::Get().SaveLevelToFile(m_CurrentLevelPath);
		//			}
		//		}
		//		if (ImGui::MenuItem("Save level as", "CTRL+ALT+S", false, bEnableLoadingOrSaving))
		//		{
		//			nfdchar_t* outPath = nullptr;
		//			nfdresult_t result = NFD_SaveDialog("zlevel", nullptr, &outPath);
		//			if (result == NFD_OKAY)
		//			{
		//				Level::Get().SaveLevelToFile(std::string(outPath));
		//				m_CurrentLevelPath = outPath;
		//				m_CurrentLevelName = std::filesystem::path(outPath).filename().string();
		//				free(outPath);
		//			}
		//			else if (result == NFD_ERROR)
		//			{
		//				ZE_CORE_ERROR("Save level failed: {0}", NFD_GetError());
		//			}
		//		}
		//		if (ImGui::MenuItem("Exit"))
		//		{
		//			// TODO: Check if current level needs saving
		//			Application::Get().Close();
		//		}
		//		ImGui::EndMenu();
		//	}
		//	// TODO: Edit menu
		//	if (ImGui::BeginMenu("Edit"))
		//	{
		//		if (ImGui::MenuItem("Undo", "CTRL+Z"))
		//		{

		//		}
		//		if (ImGui::MenuItem("Redo", "CTRL+Y", false, false))
		//		{

		//		}
		//		ImGui::Separator();
		//		if (ImGui::MenuItem("Copy", "CTRL+C"))
		//		{

		//		}
		//		if (ImGui::MenuItem("Paste", "CTRL+V"))
		//		{

		//		}
		//		if (ImGui::MenuItem("Cut", "CTRL+X"))
		//		{

		//		}
		//		ImGui::Separator();
		//		ImGui::MenuItem("Preferences", nullptr, &bShowPreferences);
		//		ImGui::EndMenu();
		//	}
		//	// Window menu
		//	if (ImGui::BeginMenu("Window"))
		//	{
		//		ImGui::MenuItem(GAME_VIEW_NAME, nullptr, &bShowGameView);
		//		ImGui::MenuItem(SCENE_OUTLINE_NAME, nullptr, &bShowSceneOutline);
		//		ImGui::MenuItem("Object Inspector", nullptr, &bShowObjectInspector);
		//		ImGui::MenuItem("Class Browser", nullptr, &bShowClassBrowser);
		//		ImGui::MenuItem("Console", nullptr, &bShowConsole);
		//		ImGui::MenuItem("Stats", nullptr, &bShowStats);
		//		ImGui::MenuItem("Particle Editor", nullptr, &m_bShowParticleEditor);
		//		ImGui::Separator();
		//		// Reset layout on next frame
		//		if (ImGui::MenuItem("Reset layout", nullptr))
		//		{
		//			m_bResetLayout = true;
		//		}
		//		ImGui::EndMenu();
		//	}
		//	// Help
		//	if (ImGui::BeginMenu("Help"))
		//	{
		//		ImGui::Separator();
		//		ImGui::MenuItem("About ZeoEngine", nullptr, &bShowAbout);
		//		ImGui::EndMenu();
		//	}
		//	// Display level file name at center of menu bar
		//	{
		//		ImGui::TextCentered("%s", m_CurrentLevelName.empty() ? "Untitled" : m_CurrentLevelName.c_str());
		//	}
		//	// Display engine stats at right corner of menu bar
		//	{
		//		const float statsWidth = 125.0f;
		//		ImGui::Indent(ImGui::GetWindowSize().x - statsWidth);
		//		ImGui::Text("%.f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate);
		//	}
		//	ImGui::EndMainMenuBar();
		//}

		////////////////////////////////////////////////////////////////////////////
		//// Editor Windows ////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////

		//if (bShowGameView)
		//{
		//	m_GameViewportPanel.OnImGuiRender(&bShowGameView);
		//}
		//else
		//{
		//	// TODO: Disable framebuffer rendering

		//}
		//if (bShowSceneOutline)
		//{
		//	ShowLevelOutline(&bShowSceneOutline);
		//}
		//if (bShowObjectInspector)
		//{
		//	ShowObjectInspector(&bShowObjectInspector);
		//}
		//if (bShowClassBrowser)
		//{
		//	ShowClassBrowser(&bShowClassBrowser);
		//}

		//if (m_bShowParticleEditor)
		//{
		//	CreateParticleEditorDockspace(&m_bShowParticleEditor);
		//	ShowParticleEditor(&m_bShowParticleEditor);
		//}

		//if (bShowStats)
		//{
		//	ShowStats(&bShowStats);
		//}

		//if (bShowPreferences)
		//{
		//	ShowPreferences(&bShowPreferences);
		//}

		//if (bShowAbout)
		//{
		//	ShowAbout(&bShowAbout);
		//}

		//// Put it at last to prevent GameObject's UI from being covered by GameView
		//Level::Get().OnImGuiRender();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(EditorLayer::OnKeyPressed));

		m_DockspaceManager.OnEvent(event);

		//if (m_bIsHoveringViews[GAME_VIEW] && pieState == PIEState::None)
		//{
		//	m_CameraControllers[GAME_VIEW]->OnEvent(event);
		//}
		//if (m_bIsHoveringViews[PARTICLE_VIEW])
		//{
		//	m_CameraControllers[PARTICLE_VIEW]->OnEvent(event);
		//}

	}

	void EditorLayer::PushDockspace(EditorDockspace* dockspace)
	{
		m_DockspaceManager.PushDockspace(dockspace);
	}

	void EditorLayer::ShowLevelOutline(bool* bShow)
	{
		auto& level = Level::Get();

		if (ImGui::Begin("Level Outline", bShow))
		{
			static uint32_t objectCount = 0;
			ImGui::Text("(%d objects total)", objectCount);
			objectCount = 0;
			for (auto it = level.m_SortedGameObjects.cbegin(); it != level.m_SortedGameObjects.cend(); ++it)
			{
				if (it->second->IsPendingDestroy())
					continue;

				++objectCount;

				ImVec4 color;
				if (it->second->IsActive())
				{
					// Dynamically spawned objects are marked yellow instead of white
					color = it->second->IsDynamicallySpawned() ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				else
				{
					// Inactive objects are marked darker
					color = it->second->IsDynamicallySpawned() ? ImVec4(0.75f, 0.75f, 0.0f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
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
				// Target should be exposed GameObject* variables in Object Inspector window
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

	void EditorLayer::ShowObjectInspector(bool* bShow)
	{
		if (ImGui::Begin("Object Inspector", bShow))
		{
			if (m_SelectedGameObject && !m_SelectedGameObject->IsPendingDestroy())
			{
				m_CurrentPropertySource = GAMEOBJECT_PROP;
				if (m_bIsSortedPropertiesDirty[m_CurrentPropertySource])
				{
					PreProcessProperties(m_SelectedGameObject);
				}
				PropertyData data;
				data.bPropertyRecursed = false;
				// NOTE: Do not combine following two statements into "data.Object = data.OutermostObject = &rttr::instance(m_SelectedGameObject);",
				// which will get optimized out in release mode causing data.Object to be an invalid object
				rttr::instance object = m_SelectedGameObject;
				data.Object = data.OutermostObject = &object;
				ProcessPropertiesRecursively(data);
			}
		}
		ImGui::End();
	}

	void EditorLayer::PreProcessProperties(rttr::instance object)
	{
		m_SortedProperties[m_CurrentPropertySource].clear();
		rttr::instance obj = object.get_type().get_raw_type().is_wrapper() ? object.get_wrapped_instance() : object;
		// Get the most derived class's properties
		auto properties = obj.get_derived_type().get_properties();
		for (auto prop : properties)
		{
			// Properties with Hidden meta do not pass here
			if (prop.get_metadata(PropertyMeta::Hidden))
				continue;

			rttr::variant categoryVar = prop.get_metadata(PropertyMeta::Category);
			if (categoryVar)
			{
				const std::string& category = categoryVar.get_value<std::string>();
				m_SortedProperties[m_CurrentPropertySource][category].emplace_back(prop);
			}
			else
			{
				m_SortedProperties[m_CurrentPropertySource]["Default"].emplace_back(prop);
			}
		}
		m_LoggedProperties[m_CurrentPropertySource].reserve(properties.size());
		m_bIsSortedPropertiesDirty[m_CurrentPropertySource] = false;
	}

	void EditorLayer::ProcessPropertiesRecursively(PropertyData& data)
	{
		rttr::instance obj = data.Object->get_type().get_raw_type().is_wrapper() ? data.Object->get_wrapped_instance() : *data.Object;
		// If it is the outermost property
		if (!data.bPropertyRecursed)
		{
			for (const auto& [category, properties] : m_SortedProperties[m_CurrentPropertySource])
			{
				ImGui::Columns(1);
				// Display category seperator
				if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					for (auto prop : properties)
					{
						data.Property = &prop;
						data.Object = &obj;
						ProcessProperty(data);
					}
				}
			}
		}
		// TODO: Category of child properties
		// Or it is a child property of a class/struct or possibly an element of a container
		else
		{
			if (obj.get_type().is_derived_from<GameObject>())
			{
				LogPropertyMessage(*data.Property, "Failed registering property {0}! You should always register it with base type GameObject* and specify PropertyMeta::SubclassOf instead. See comments of PropertyMeta::SubclassOf for details.", 3);
				return;
			}
			// Get the most derived class's properties
			auto properties = obj.get_derived_type().get_properties();
			// Note: If custom struct/class is a pointer, it must not be null in order to retrieve its properties
			// @see GameObject::m_Transform, GameObject::m_CollisionData
			for (auto prop : properties)
			{
				data.Property = &prop;
				data.Object = &obj;
				ProcessProperty(data);
			}
			data.bPropertyRecursed = false;
		}
	}

	void EditorLayer::ProcessProperty(PropertyData& data)
	{
		rttr::variant var = data.Property->get_value(*data.Object);
		if (!var)
			return;

		data.PropertyValue = &var;

		// Do not show this property if "HideCondition" meets
		rttr::variant hideConditionVar = data.Property->get_metadata(PropertyMeta::HideCondition);
		// If current property has defined "HideCondition" metadata, attempt to find the required data to evaluate in the processed map; if failed, process it
		if (hideConditionVar)
		{
			auto it = m_HideConditionProperties[m_CurrentPropertySource].find(*data.Property);
			if (it != m_HideConditionProperties[m_CurrentPropertySource].cend())
			{
				if (it->second.first.get_value(*data.Object).to_string() == it->second.second)
				{
					return;
				}
			}
			else
			{
				bool bOK = true;
				std::string hideConditionStr = hideConditionVar.to_string(&bOK);
				if (bOK)
				{
					size_t seperatorPos = hideConditionStr.find("==");
					if (seperatorPos != std::string::npos)
					{
						std::string keyName = hideConditionStr.substr(0, seperatorPos);
						std::string valueName = hideConditionStr.substr(seperatorPos + 2, hideConditionStr.size() - 1);
						auto properties = data.Object->get_derived_type().get_properties();
						for (auto prop : properties)
						{
							// We only support bool and enum for now
							if (prop.get_type() == rttr::type::get<bool>() || prop.is_enumeration())
							{
								if (prop.get_name() == keyName)
								{
									// Cache the required data to be used the next time
									m_HideConditionProperties[m_CurrentPropertySource].emplace(std::make_pair(*data.Property, std::make_pair(prop, std::move(valueName))));
									if (prop.get_value(*data.Object).to_string() == valueName)
									{
										return;
									}
								}
							}
						}
					}
					else
					{
						bOK = false;
					}
				}
				if (!bOK)
				{
					LogPropertyMessage(*data.Property, "Syntax error on PropertyMeta::HideCondition of property {0}! See comments of PropertyMeta::HideCondition for details.", 3);
				}
			}
		}

		if (!ProcessPropertyValue(data))
		{
			LogPropertyMessage(*data.Property, "Failed to resolve value of property {0}!", 3);
		}
	}

	bool EditorLayer::ProcessPropertyValue(PropertyData& data)
	{
		rttr::type valueType = data.PropertyValue->get_type();
		rttr::type wrappedType = valueType.is_wrapper() ? valueType.get_wrapped_type() : valueType;
		bool bIsWrapper = wrappedType != valueType;
		// Backup variant value
		rttr::variant& propertyValue = *data.PropertyValue;
		data.PropertyValue = bIsWrapper ? &data.PropertyValue->extract_wrapped_value() : data.PropertyValue;
		// Atomic types
		if (ProcessAtomicTypes(wrappedType, data))
		{
		}
		else
		{
			// Restore variant value
			data.PropertyValue = &propertyValue;
			// Backup object and property
			rttr::instance& object = *data.Object;
			rttr::property& prop = *data.Property;

			// Sequential container types - this is not recognized as wrapper types
			if (propertyValue.is_sequential_container())
			{
				if (!wrappedType.is_pointer())
				{
					LogPropertyMessage(*data.Property, "Avoid registering sequential container {0} by value. Try 'policy::prop::bind_as_ptr'!", 2);
				}
				data.SequentialView = &propertyValue.create_sequential_view();
				if (data.SequentialView->get_rank() > 1)
				{
					LogPropertyMessage(*data.Property, "Registering a nested sequential container {0} is not supported!", 3);
					return true;
				}
				bool bOpen = ImGui::TreeNodeEx(data.Property->get_name().data());
				ShowPropertyTooltip(*data.Property);
				if (bOpen)
				{
					// Switch to the right column
					ImGui::NextColumn();
					// Add insert and erase-all buttons
					AddSequentialContainerButtons(data);
					// Switch to next line's left column
					ImGui::NextColumn();
					ProcessSequentialContainerTypes(data);
					// Write updated value to sequential container property
					prop.set_value(object, propertyValue);
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
			else if (propertyValue.is_associative_container())
			{
				LogPropertyMessage(*data.Property, "Registering associative containers is not supported!", 3);
				//if (ImGui::TreeNodeEx(data.Property->get_name().data()))
				//{
				//	data.AssociativeView = &propertyValue.create_associative_view();
				//	ProcessAssociativeContainerTypes(data);
				//	prop.set_value(object, propertyValue);
				//	ImGui::TreePop();
				//}
			}
			// class/struct types
			else
			{
				auto childProps = wrappedType.get_properties();
				if (!childProps.empty())
				{
					if (!wrappedType.is_pointer())
					{
						LogPropertyMessage(*data.Property, "Registering custom struct {0} by value is not well supported for some reasons. Try 'policy::prop::bind_as_ptr'!", 2);
					}
					ImGui::AlignTextToFramePadding();
					bool bOpen = ImGui::TreeNodeEx(data.Property->get_name().data(), ImGuiTreeNodeFlags_DefaultOpen);
					ShowPropertyTooltip(*data.Property);
					// Switch to next line's left column
					ImGui::NextColumn();
					ImGui::NextColumn();
					if (bOpen)
					{
						data.bPropertyRecursed = true;
						data.OuterProperty = data.Property;
						rttr::instance object = propertyValue;
						data.Object = &object;
						ProcessPropertiesRecursively(data);
						ImGui::TreePop();
					}
				}
				else
				{
					return false;
				}
			}
		}
		return true;
	}

	bool EditorLayer::ProcessAtomicTypes(const rttr::type& type, PropertyData& data)
	{
		if (type.is_arithmetic())
		{
			if (type == rttr::type::get<bool>())
			{
				ProcessBoolType(data.PropertyValue->to_bool(), data);
			}
			else if (type == rttr::type::get<int8_t>())
			{
				
			}
			else if (type == rttr::type::get<int32_t>())
			{
				
			}
			else if (type == rttr::type::get<int64_t>())
			{
				
			}
			else if (type == rttr::type::get<uint8_t>())
			{
				
			}
			else if (type == rttr::type::get<uint32_t>())
			{
				
			}
			else if (type == rttr::type::get<uint64_t>())
			{
				
			}
			else if (type == rttr::type::get<float>())
			{
				
			}
			else if (type == rttr::type::get<double>())
			{
				
			}
			return true;
		}
		// enum
		else if (type.is_enumeration())
		{
			ProcessEnumType(data);
			return true;
		}
		// std::string
		else if (type.get_raw_type() == rttr::type::get<std::string>())
		{
			if (data.SequentialIndex == -1)
			{
				if (!type.is_pointer())
				{
					LogPropertyMessage(*data.Property, "Registering std::string {0} by value is not supported. Try 'policy::prop::bind_as_ptr'!", 3);
				}
				else
				{
					ProcessStringType(data.PropertyValue->get_value<std::string*>(), data);
				}
			}
			else
			{
				ProcessStringType(&data.PropertyValue->get_value<std::string>(), data);
			}
			return true;
		}
		// glm::i32vec2
		else if (type.get_raw_type() == rttr::type::get<glm::i32vec2>())
		{
			if (data.SequentialIndex == -1)
			{
				if (!type.is_pointer())
				{
					LogPropertyMessage(*data.Property, "Registering glm::i32vec2 {0} by value is not supported. Try 'policy::prop::bind_as_ptr'!", 3);
				}
				else
				{
					ProcessI32Vec2Type(data.PropertyValue->get_value<glm::i32vec2*>(), data);
				}
			}
			else
			{
				ProcessI32Vec2Type(&data.PropertyValue->get_value<glm::i32vec2>(), data);
			}
			return true;
		}
		// glm::vec2
		else if (type.get_raw_type() == rttr::type::get<glm::vec2>())
		{
			if (data.SequentialIndex == -1)
			{
				if (!type.is_pointer())
				{
					LogPropertyMessage(*data.Property, "Registering glm::vec2 {0} by value is not supported. Try 'policy::prop::bind_as_ptr'!", 3);
				}
				else
				{
					ProcessVec2Type(data.PropertyValue->get_value<glm::vec2*>(), data);
				}
			}
			else
			{
				ProcessVec2Type(&data.PropertyValue->get_value<glm::vec2>(), data);
			}
			return true;
		}
		// glm::vec3
		else if (type.get_raw_type() == rttr::type::get<glm::vec3>())
		{
			if (data.SequentialIndex == -1)
			{
				if (!type.is_pointer())
				{
					LogPropertyMessage(*data.Property, "Registering glm::vec3 {0} by value is not supported. Try 'policy::prop::bind_as_ptr'!", 3);
				}
				else
				{
					ProcessVec3Type(data.PropertyValue->get_value<glm::vec3*>(), data);
				}
			}
			else
			{
				ProcessVec3Type(&data.PropertyValue->get_value<glm::vec3>(), data);
			}
			return true;
		}
		// glm::vec4
		else if (type.get_raw_type() == rttr::type::get<glm::vec4>())
		{
			if (data.SequentialIndex == -1)
			{
				if (!type.is_pointer())
				{
					LogPropertyMessage(*data.Property, "Registering glm::vec4 {0} by value is not supported. Try 'policy::prop::bind_as_ptr'!", 3);
				}
				else
				{
					ProcessColorType(data.PropertyValue->get_value<glm::vec4*>(), data);
				}
			}
			else
			{
				ProcessColorType(&data.PropertyValue->get_value<glm::vec4>(), data);
			}
			return true;
		}
		// GameObject*
		else if (type == rttr::type::get<GameObject*>())
		{
			ProcessGameObjectType(data.PropertyValue->get_value<GameObject*>(), data);
			return true;
		}
		// Ref<Texture2D>
		else if (type.get_raw_type() == rttr::type::get<Texture2D>())
		{
			return true;
		}
		// ParticleSystem*
		else if (type.get_raw_type() == rttr::type::get<ParticleSystem>())
		{
			if (!type.is_pointer())
			{
				LogPropertyMessage(*data.Property, "Registering ParticleSystem type {0} is not supported. Try ParticleSystem*!", 3);
			}
			else
			{
				ProcessParticleSystemType(data.PropertyValue->get_value<ParticleSystem*>(), data);
			}
			return true;
		}

		return false;
	}

	void EditorLayer::AddSequentialContainerButtons(const PropertyData& data)
	{
		rttr::type itemType = data.SequentialView->get_value_type();
		std::stringstream ss;
		ss << "##" << data.Property->get_name() << data.SequentialView->get_value_type().get_name();
		if (ImGui::BeginCombo(ss.str().c_str(), nullptr, ImGuiComboFlags_NoPreview))
		{
			ss.clear(); ss.str(""); ss << "Insert##" << data.Property->get_name() << data.SequentialView->get_value_type().get_name();
			// Insert a new item at the end
			if (ImGui::Selectable(ss.str().c_str()))
			{
				// TODO: We have to add every supported type explicitly for sequential container insertion, which is really bad

				if (itemType == rttr::type::get<bool>())
				{
					// Note: std::vector<bool> is not supported because stl uses bits to store bool elements internally
					if (data.Property->get_type().get_raw_type() == rttr::type::get<std::vector<bool>>())
					{
						ZE_CORE_ERROR("std::vector<bool> sequential container {0} is not supported! Try other sequential containers for bool instead.", data.Property->get_name());
					}
					else
					{
						data.SequentialView->insert(data.SequentialView->end(), false);
					}
				}
				else if (itemType == rttr::type::get<int8_t>())
				{
					data.SequentialView->insert(data.SequentialView->end(), 0i8);
				}
				else if (itemType == rttr::type::get<int32_t>())
				{
					data.SequentialView->insert(data.SequentialView->end(), 0i32);
				}
				else if (itemType == rttr::type::get<int64_t>())
				{
					data.SequentialView->insert(data.SequentialView->end(), 0i64);
				}
				else if (itemType == rttr::type::get<uint8_t>())
				{
					data.SequentialView->insert(data.SequentialView->end(), 0ui8);
				}
				else if (itemType == rttr::type::get<uint32_t>())
				{
					data.SequentialView->insert(data.SequentialView->end(), 0ui32);
				}
				else if (itemType == rttr::type::get<uint64_t>())
				{
					data.SequentialView->insert(data.SequentialView->end(), 0ui64);
				}
				else if (itemType == rttr::type::get<float>())
				{
					data.SequentialView->insert(data.SequentialView->end(), 0.0f);
				}
				else if (itemType == rttr::type::get<double>())
				{
					data.SequentialView->insert(data.SequentialView->end(), 0.0);
				}
				else if (itemType.is_enumeration())
				{
					data.SequentialView->insert(data.SequentialView->end(), *itemType.get_enumeration().get_values().cbegin());
				}
				else if (itemType == rttr::type::get<std::string>())
				{
					data.SequentialView->insert(data.SequentialView->end(), std::string(""));
				}
				else if (itemType == rttr::type::get<glm::vec2>())
				{
					data.SequentialView->insert(data.SequentialView->end(), glm::vec2(0.0f));
				}
				else if (itemType == rttr::type::get<glm::i32vec2>())
				{
					data.SequentialView->insert(data.SequentialView->end(), glm::i32vec2(0i32));
				}
				else if (itemType == rttr::type::get<glm::vec3>())
				{
					data.SequentialView->insert(data.SequentialView->end(), glm::vec3(0.0f));
				}
				else if (itemType == rttr::type::get<glm::vec4>())
				{
					data.SequentialView->insert(data.SequentialView->end(), glm::vec4(0.0f));
				}
				else if (itemType == rttr::type::get<GameObject*>())
				{
					data.SequentialView->insert(data.SequentialView->end(), static_cast<GameObject*>(nullptr));
				}
				else if (itemType == rttr::type::get<Ref<Texture2D>>())
				{
					data.SequentialView->insert(data.SequentialView->end(), Ref<Texture2D>());
				}
				else if (itemType == rttr::type::get<BurstData>())
				{
					data.SequentialView->insert(data.SequentialView->end(), BurstData());
				}
				else if (itemType == rttr::type::get<ParticleSystem*>())
				{
					data.SequentialView->insert(data.SequentialView->end(), static_cast<ParticleSystem*>(nullptr));
				}
				else
				{
					ZE_CORE_WARN("Unknown sequential container {0}!", data.Property->get_name());
				}
			}	
			if (!data.SequentialView->is_empty())
			{
				ss.clear(); ss.str(""); ss << "Erase all##" << data.Property->get_name() << data.SequentialView->get_value_type().get_name();
				// Clear all items
				if (ImGui::Selectable(ss.str().c_str()))
				{
					data.SequentialView->clear();
					InvokePropertyChangeCallback(data);
				}
			}
			ImGui::EndCombo();
		}
	}

	void EditorLayer::ProcessSequentialContainerTypes(PropertyData& data)
	{
		for (uint32_t i = 0; i < data.SequentialView->get_size(); ++i)
		{
			rttr::variant item = data.SequentialView->get_value(i);
			// TODO: Nested sequential containers not supported
			if (item.is_sequential_container())
			{
				//std::stringstream ss;
				//ss << "[" << i << "]";
				//if (ImGui::TreeNodeEx(ss.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				//{
				//	data.SequentialView = &item.create_sequential_view();
				//	ProcessSequentialContainerTypes(data);
				//	ImGui::TreePop();
				//}
			}
			else
			{
				rttr::variant wrappedVar = item.extract_wrapped_value();
				rttr::type valueType = wrappedVar.get_type();
				rttr::type wrappedType = valueType.is_wrapper() ? valueType.get_wrapped_type() : valueType;
				bool bIsWrapper = wrappedType != valueType;
				// Backup variant value
				rttr::variant& propertyValue = *data.PropertyValue;
				data.PropertyValue = bIsWrapper ? &wrappedVar.extract_wrapped_value() : &wrappedVar;
				data.SequentialIndex = i;
				if (ProcessAtomicTypes(wrappedType, data))
				{
				}
				// TODO: Sequential containers of custom struct/class not supported for now because we cannot implement a generic strongly typed insertion function
				// Currently for internal use currently
				else
				{
					// Restore variant value
					data.PropertyValue = &propertyValue;

					std::stringstream ss;
					ss << "[" << i << "]";
					if (ImGui::TreeNodeEx(ss.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
					{
						// Switch to the right column
						ImGui::NextColumn();
						// Add insert and erase buttons
						{
							ss.clear(); ss.str(""); ss << "##" << data.Property->get_name() << data.SequentialView->get_value_type().get_name() << data.SequentialIndex;
							if (ImGui::BeginCombo(ss.str().c_str(), nullptr, ImGuiComboFlags_NoPreview))
							{
								ss.clear(); ss.str(""); ss << "Insert##" << data.Property->get_name() << data.SequentialView->get_value_type().get_name() << data.SequentialIndex;
								if (ImGui::Selectable(ss.str().c_str()))
								{
									// Item type is BurstData
									if (data.SequentialView->get_value_type() == rttr::type::get<BurstData>())
									{
										// Increment SequentialIndex after insertion to match data.Object
										data.SequentialView->insert(data.SequentialView->begin() + data.SequentialIndex++, BurstData());
									}
								}
								ss.clear(); ss.str(""); ss << "Erase##" << data.Property->get_name() << data.SequentialView->get_value_type().get_name() << data.SequentialIndex;
								if (ImGui::Selectable(ss.str().c_str()))
								{
									data.SequentialView->erase(data.SequentialView->begin() + data.SequentialIndex);
									InvokePropertyChangeCallback(data);
									// The last item has been erased, current sequential container has finished processing
									if (data.SequentialIndex >= data.SequentialView->get_size())
									{
										data.SequentialIndex = -1;
										ImGui::EndCombo();
										ImGui::TreePop();
										return;
									}
									// Because previous item is erased, we need to update item (data.Object) to match SequentialIndex
									else
									{
										wrappedVar = data.SequentialView->get_value(data.SequentialIndex).extract_wrapped_value();
									}
								}
								ImGui::EndCombo();
							}
						}
						// Switch to next line's left column
						ImGui::NextColumn();
						data.bPropertyRecursed = true;
						data.OuterProperty = data.Property;
						rttr::instance object = wrappedVar;
						data.Object = &object;
						// Backup current property
						rttr::property& prop = *data.Property;
						// Backup current outer property
						rttr::property& outerProp = *data.OuterProperty;
						ProcessPropertiesRecursively(data);
						// Restore property
						data.Property = &prop;
						// Restore outer property
						data.OuterProperty = &outerProp;
						ImGui::TreePop();
					}
					// Current sequential container has finished processing, reset SequentialIndex
					data.SequentialIndex = -1;
				}
			}
		}
	}

	void EditorLayer::ProcessAssociativeContainerTypes(PropertyData& data)
	{
		// i.e. std::set
		if (data.AssociativeView->is_key_only_type())
		{
			for (auto& [key, value] : *data.AssociativeView)
			{
				data.PropertyValue = const_cast<rttr::variant*>(&key);
				ProcessPropertyValue(data);
			}
		}
		else
		{
			for (auto& [key, value] : *data.AssociativeView)
			{
				data.PropertyValue = const_cast<rttr::variant*>(&key);
				ProcessPropertyValue(data);
				ImGui::SameLine();
				data.PropertyValue = const_cast<rttr::variant*>(&value);
				ProcessPropertyValue(data);
			}
		}
	}

	void EditorLayer::LogPropertyMessage(const rttr::property& prop, const char* msg, uint32_t logLevel)
	{
		auto [it, res] = m_LoggedProperties[m_CurrentPropertySource].emplace(prop);
		if (res)
		{
			switch (logLevel)
			{
			case 0:
				ZE_CORE_TRACE(msg, prop.get_name());
				break;
			case 1:
				ZE_CORE_INFO(msg, prop.get_name());
				break;
			case 2:
				ZE_CORE_WARN(msg, prop.get_name());
				break;
			case 3:
				ZE_CORE_ERROR(msg, prop.get_name());
				break;
			case 4:
				ZE_CORE_CRITICAL(msg, prop.get_name());
				break;
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
				for (const auto& [parentType, childTypes] : m_ClassInheritanceTree)
				{
					// Class iteration starts from GameObject type
					if (parentType.get_name() == "GameObject")
					{
						DisplayClassHierarchyRecursively(childTypes);
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}

	void EditorLayer::CreateParticleEditorDockspace(bool* bShow)
	{
		//ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
		//SetNextWindowDefaultPosition();
		//ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		//ImGui::Begin("Particle Editor", bShow, windowFlags);
		//ImGui::PopStyleVar(2);
		//ImGuiID particleEditorDockspaceId = ImGui::GetID("ParticleEditorDockspace");
		//// Update docking layout
		//if (ImGui::DockBuilderGetNode(particleEditorDockspaceId) == nullptr || m_bResetLayout)
		//{
		//	m_bResetLayout = false;

		//	// Clear out existing layout
		//	ImGui::DockBuilderRemoveNode(particleEditorDockspaceId);
		//	// Add empty node
		//	ImGui::DockBuilderAddNode(particleEditorDockspaceId, ImGuiDockNodeFlags_DockSpace);
		//	// Main node should cover entire window
		//	ImGui::DockBuilderSetNodeSize(particleEditorDockspaceId, ImGui::GetWindowSize());

		//	ImGuiID dockMainLeft;
		//	ImGuiID dockMainRight = ImGui::DockBuilderSplitNode(particleEditorDockspaceId, ImGuiDir_Right, 0.5f, nullptr, &dockMainLeft);

		//	ImGui::DockBuilderDockWindow("Particle View", dockMainLeft);
		//	ImGui::DockBuilderDockWindow("Particle Inspector", dockMainRight);

		//	ImGui::DockBuilderFinish(particleEditorDockspaceId);
		//}
		//// Should be put at last
		//ImGui::DockSpace(particleEditorDockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
		//ImGui::End();
	}

	void EditorLayer::ShowParticleEditor(bool* bShow)
	{
		if (ImGui::Begin("Particle Editor", bShow))
		{
			if (ImGui::BeginMenuBar())
			{
				// File menu
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New particle system"))
					{
						m_EditorParticleSystem = ParticleSystem::CreateDefaultParticleSystem();
						m_CurrentParticleSystemPath.clear();
						m_CurrentParticleSystemName.clear();
					}
					if (ImGui::MenuItem("Load particle system"))
					{
						nfdchar_t* outPath = nullptr;
						nfdresult_t result = NFD_OpenDialog("zparticle", nullptr, &outPath);
						if (result == NFD_OKAY)
						{
							LoadParticleSystemFromFile(outPath);
							m_CurrentParticleSystemPath = outPath;
							m_CurrentParticleSystemName = std::filesystem::path(outPath).filename().string();
							free(outPath);
						}
						else if (result == NFD_ERROR)
						{
							ZE_CORE_ERROR("Load particle system failed: {0}", NFD_GetError());
						}
					}
					if (ImGui::MenuItem("Save particle system"))
					{
						if (m_CurrentParticleSystemPath.empty())
						{
							nfdchar_t* outPath = nullptr;
							nfdresult_t result = NFD_SaveDialog("zparticle", nullptr, &outPath);
							if (result == NFD_OKAY)
							{
								std::string pathStr = outPath;
								free(outPath);
								SaveParticleSystemToFile(pathStr);
								m_CurrentParticleSystemPath = pathStr;
								static const char* particleSystemFileSuffix = ".zparticle";
								if (pathStr.rfind(particleSystemFileSuffix) == std::string::npos)
								{
									pathStr += particleSystemFileSuffix;
								}
								m_CurrentParticleSystemName = std::filesystem::path(pathStr).filename().string();
							}
							else if (result == NFD_ERROR)
							{
								ZE_CORE_ERROR("Save particle system failed: {0}", NFD_GetError());
							}
						}
						else
						{
							SaveParticleSystemToFile(m_CurrentParticleSystemPath);
						}
					}
					ImGui::EndMenu();
				}
				// Display particle system file name at center of menu bar
				{
					ImGui::TextCentered("%s", m_CurrentParticleSystemName.empty() ? "Untitled" : m_CurrentParticleSystemName.c_str());
				}
				ImGui::EndMenuBar();
			}

			if (ImGui::Begin("Particle View", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				//if (!m_CameraControllers[PARTICLE_VIEW])
				//{
				//	const auto& window = Application::Get().GetWindow();
				//	m_CameraControllers[PARTICLE_VIEW] = CreateScope<OrthographicCameraController>(static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight()));
				//	m_CameraControllers[PARTICLE_VIEW]->SetZoomLevel(3.0f);
				//}

				if (!m_EditorParticleSystem)
				{
					m_EditorParticleSystem = ParticleSystem::CreateDefaultParticleSystem();
				}

				ImGuiWindow* window = ImGui::GetCurrentWindow();
				glm::vec2 max = { window->InnerRect.Max.x, window->InnerRect.Max.y };
				glm::vec2 min = { window->InnerRect.Min.x, window->InnerRect.Min.y };
				glm::vec2 size = max - min;
				if (size != m_LastParticleViewSize)
				{
					//OnParticleViewWindowResized(size);
				}
				// Draw framebuffer texture
				//ImGui::Image(m_FBOs[PARTICLE_VIEW]->GetColorAttachment(),
				//	ImVec2(window->InnerRect.Max.x - window->InnerRect.Min.x, window->InnerRect.Max.y - window->InnerRect.Min.y),
				//	ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				//max = { window->InnerRect.Max.x, window->InnerRect.Max.y };
				//min = { window->InnerRect.Min.x, window->InnerRect.Min.y };
				//m_LastParticleViewSize = max - min;
				//m_bIsHoveringViews[PARTICLE_VIEW] = ImGui::IsItemHovered();

				if (m_EditorParticleSystem)
				{
					m_EditorParticleSystem->OnParticleViewImGuiRender();
				}
			}
			ImGui::End();

			if (ImGui::Begin("Particle Inspector"))
			{
				if (m_EditorParticleSystem)
				{
					m_CurrentPropertySource = PARTICLESYSTEM_PROP;
					if (m_bIsSortedPropertiesDirty[m_CurrentPropertySource])
					{
						PreProcessProperties(m_EditorParticleSystem);
					}
					PropertyData data;
					data.bPropertyRecursed = false;
					rttr::instance object = m_EditorParticleSystem;
					data.Object = data.OutermostObject = &object;
					ProcessPropertiesRecursively(data);
				}
			}
			ImGui::End();
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
			if (pieState == PIEState::None)
			{
				StartPIE();
			}
		}
		// Exit PIE by pressing Esc
		if (Input::IsKeyPressed(ZE_KEY_ESCAPE))
		{
			if (pieState != PIEState::None)
			{
				StopPIE();
			}
		}
		return false;
	}

	void EditorLayer::OnGameObjectSelectionChanged(GameObject* lastSelectedGameObject)
	{
		m_bIsSortedPropertiesDirty[GAMEOBJECT_PROP] = true;
		m_LoggedProperties[GAMEOBJECT_PROP].clear();
		m_HideConditionProperties[GAMEOBJECT_PROP].clear();
	}

	void EditorLayer::OnGameViewImGuiRender()
	{
		// Do not draw in PIE mode
		if (pieState == PIEState::None)
		{
			EditTransform();
			DrawCollision();
		}
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

		//if (m_bIsHoveringViews[GAME_VIEW] && ImGui::IsKeyPressed(ZE_KEY_W))
		//{
		//	currentGizmoOperation = ImGuizmo::TRANSLATE;
		//}
		//if (m_bIsHoveringViews[GAME_VIEW] && ImGui::IsKeyPressed(ZE_KEY_E))
		//{
		//	currentGizmoOperation = ImGuizmo::ROTATE;
		//}
		//if (m_bIsHoveringViews[GAME_VIEW] && ImGui::IsKeyPressed(ZE_KEY_R))
		//{
		//	currentGizmoOperation = ImGuizmo::SCALE;
		//}
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 5.0f, 5.0f });
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
			//if (m_bIsHoveringViews[GAME_VIEW] && ImGui::IsKeyPressed(ZE_KEY_S))
			//{
			//	bUseSnap = !bUseSnap;
			//}
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
		ImGui::PopStyleVar();

		// Draw transform gizmo on the selected GameObject when not in PIE
		// as editor camera, which gizmo needs, will get deactivated in PIE
		if (m_SelectedGameObject && !m_SelectedGameObject->IsPendingDestroy())
		{
			ImGuizmo::SetDrawlist();
			// TODO: Not applied to 3D rendering
			ImGuizmo::SetOrthographic(true);

			ImGuiWindow* window = ImGui::GetCurrentWindow();
			ImGuizmo::SetRect(window->InnerRect.Min.x, window->InnerRect.Min.y, window->InnerRect.GetSize().x, window->InnerRect.GetSize().y);

			//ImGuizmo::Manipulate(glm::value_ptr(m_CameraControllers[GAME_VIEW]->GetCamera().GetViewMatrix()), glm::value_ptr(m_CameraControllers[GAME_VIEW]->GetCamera().GetProjectionMatrix()),
			//	currentGizmoOperation, currentGizmoMode,
			//	glm::value_ptr(m_SelectedGameObject->m_TransformMatrix), nullptr,
			//	bUseSnap ? &snap[0] : nullptr,
			//	bUseBoundSizing ? localBounds : nullptr,
			//	bUseBoundSizingSnap ? boundSizingSnap : nullptr);
			//m_SelectedGameObject->DecomposeTransformMatrix();
		}
	}

	void EditorLayer::DrawCollision()
	{
		if (!m_SelectedGameObject || m_SelectedGameObject->IsPendingDestroy())
			return;

		//const CollisionData* collisionData = m_SelectedGameObject->GetCollisionData();
		//const ObjectCollisionType collisionType = m_SelectedGameObject->GetCollisionType();
		//if (collisionData && collisionData->bDrawCollision)
		//{
		//	auto& gameViewCamera = m_CameraControllers[GAME_VIEW]->GetCamera();
		//	ImDrawList* dl = ImGui::GetWindowDrawList();
		//	const glm::vec2 collisionScreenCenter = ProjectWorldToScreen2D(m_SelectedGameObject->GetPosition2D() + collisionData->CenterOffset, ImGui::GetCurrentWindow(), &gameViewCamera);
		//	static const ImU32 collisionColor = IM_COL32(255, 136, 0, 255); // Orange color
		//	static const float collisionThickness = 2.5f;
		//	if (collisionType == ObjectCollisionType::Box)
		//	{
		//		const float collisionScreenExtentX = dynamic_cast<const BoxCollisionData*>(collisionData)->Extents.x / gameViewCamera.GetCameraBounds().Right * ImGui::GetCurrentWindow()->InnerRect.GetSize().x / 2;
		//		const float collisionScreenExtentY = dynamic_cast<const BoxCollisionData*>(collisionData)->Extents.y / gameViewCamera.GetCameraBounds().Top * ImGui::GetCurrentWindow()->InnerRect.GetSize().y / 2;
		//		dl->AddRect(ImVec2(collisionScreenCenter.x - collisionScreenExtentX, collisionScreenCenter.y - collisionScreenExtentY),
		//			ImVec2(collisionScreenCenter.x + collisionScreenExtentX, collisionScreenCenter.y + collisionScreenExtentY), collisionColor,
		//			0.0f, 15, collisionThickness);
		//	}
		//	else if (collisionType == ObjectCollisionType::Sphere)
		//	{
		//		const float collisionScreenRadius = dynamic_cast<const SphereCollisionData*>(collisionData)->Radius / gameViewCamera.GetCameraBounds().Right * ImGui::GetCurrentWindow()->InnerRect.GetSize().x / 2;
		//		dl->AddCircle(ImVec2(collisionScreenCenter.x, collisionScreenCenter.y), collisionScreenRadius, collisionColor, 36, collisionThickness);
		//	}
		//}
	}

	void EditorLayer::LoadParticleSystemFromFile(const char* particleSystemPath)
	{
		const std::string relativePath = ToRelativePath(particleSystemPath);
		m_EditorParticleSystem = GetParticleLibrary()->UpdateOrLoad(relativePath);
	}

	void EditorLayer::SaveParticleSystemToFile(std::string& particleSystemPath)
	{
		if (particleSystemPath.rfind(".zparticle") == std::string::npos)
		{
			particleSystemPath += ".zparticle";
		}
		std::ofstream out(particleSystemPath, std::ios::out | std::ios::binary);
		if (!out)
		{
			ZE_CORE_ERROR("Could not open particle system file!");
			return;
		}

		out << "#type " << ParticleSystem::ParticleSystemFileToken << std::endl;
		out << Serializer::Get().Serialize(m_EditorParticleSystem);
	}

	constexpr const char* PIETempFile = "temp/PIE.tmp";

	void EditorLayer::StartPIE()
	{
		// Save level to a temp file
		Level::Get().SaveLevelToFile(std::string(PIETempFile), true);

		//m_ToolBarTextures[0] = m_StopTexture;
		pieState = PIEState::Running;
		ZE_CORE_TRACE("PIE started");
	}

	void EditorLayer::StopPIE()
	{
		// Load saved temp file
		Level::Get().LoadLevelFromFile(PIETempFile, true);
		// Clear cache
		std::fstream(PIETempFile, std::ios::out);

		//m_ToolBarTextures[0] = m_PlayTexture;
		//m_ToolBarTextures[1] = m_PauseTexture;
		pieState = PIEState::None;
		ZE_CORE_TRACE("PIE stopped");
	}

	void EditorLayer::PausePIE()
	{
		//m_ToolBarTextures[1] = m_PlayTexture;
		pieState = PIEState::Paused;
		ZE_CORE_TRACE("PIE paused");
	}

	void EditorLayer::ResumePIE()
	{
		//m_ToolBarTextures[1] = m_PauseTexture;
		pieState = PIEState::Running;
		ZE_CORE_TRACE("PIE resumed");
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

	void EditorLayer::InvokePropertyChangeCallback(const PropertyData& data, bool bInvokeOnlyIfDeactivated)
	{
		data.OutermostObject->get_type().invoke("OnPropertyValueEditChange", *data.OutermostObject, { const_cast<const rttr::property*>(data.Property), const_cast<const rttr::property*>(data.OuterProperty) });
		if (bInvokeOnlyIfDeactivated)
		{
			data.OutermostObject->get_type().invoke("PostPropertyValueEditChange", *data.OutermostObject, { const_cast<const rttr::property*>(data.Property), const_cast<const rttr::property*>(data.OuterProperty) });
		}
	}

	void EditorLayer::BeginDisplayProperty(std::stringstream& ss, const PropertyData& data)
	{
		ImGui::Columns(2);
		ImGui::AlignTextToFramePadding();
		if (data.SequentialIndex == -1)
		{
			ImGui::TreeNodeEx(data.Property->get_name().data(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
			ShowPropertyTooltip(*data.Property);
			// Switch to the right column
			ImGui::NextColumn();
			// Align width to the right side
			ImGui::SetNextItemWidth(-1.0f);
			// We use property name as id here because rttr guarantees that only properties with different names can be registered
			ss << "##" << data.Property->get_name().data();
		}
		else
		{
			// Property of custom struct/class inside a sequential container
			if (data.bPropertyRecursed)
			{
				ss << data.Property->get_name().data();
			}
			// Property inside a sequential container
			else
			{
				ss << "[" << data.SequentialIndex << "]";
			}
			ImGui::TreeNodeEx(ss.str().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
			ShowPropertyTooltip(*data.Property);
			// Switch to the right column
			ImGui::NextColumn();
			ss.clear(); ss.str("");
			ss << "##" << data.Property->get_name().data() << data.SequentialIndex;
		}
	}

	// This method does not apply to custom struct/class inside a sequential container
	// @see ProcessSequentialContainerTypes()
	void EditorLayer::AddSequentialItemButtons(std::stringstream& ss, const PropertyData& data, rttr::argument insertValue)
	{
		ImGui::SameLine();
		ss.clear(); ss.str(""); ss << "##" << data.Property->get_name() << data.SequentialView->get_value_type().get_name() << data.SequentialIndex;
		if (ImGui::BeginCombo(ss.str().c_str(), nullptr, ImGuiComboFlags_NoPreview))
		{
			ss.clear(); ss.str(""); ss << "Insert##" << data.Property->get_name() << data.SequentialView->get_value_type().get_name() << data.SequentialIndex;
			// Insert a new item before current item
			if (ImGui::Selectable(ss.str().c_str()))
			{
				data.SequentialView->insert(data.SequentialView->begin() + data.SequentialIndex, insertValue);
			}
			ss.clear(); ss.str(""); ss << "Erase##" << data.Property->get_name() << data.SequentialView->get_value_type().get_name() << data.SequentialIndex;
			// Erase current item
			if (ImGui::Selectable(ss.str().c_str()))
			{
				data.SequentialView->erase(data.SequentialView->begin() + data.SequentialIndex);
			}
			ImGui::EndCombo();
		}
	}

	void EditorLayer::SetPropertyValue(const PropertyData& data, rttr::argument value, rttr::argument insertValue, RecursedSequentialPropertyProcessingFn func)
	{
		if (data.SequentialIndex == -1)
		{
			data.Property->set_value(*data.Object, value);
		}
		else
		{
			// Property inside a sequential container
			if (!data.bPropertyRecursed)
			{
				data.SequentialView->set_value(data.SequentialIndex, value);
			}
			// Call the custom set-value function if provided, usually for properties of custom struct/class inside a sequential container
			else if (func)
			{
				func(data);
			}
		}
	}

	void EditorLayer::EndDisplayProperty(std::stringstream& ss, const PropertyData& data, rttr::argument value, rttr::argument insertValue, RecursedSequentialPropertyProcessingFn func)
	{
		ShowPropertyTooltip(*data.Property);
		if (data.SequentialIndex == -1)
		{
			data.Property->set_value(*data.Object, value);
		}
		else
		{
			// Property inside a sequential container
			if (!data.bPropertyRecursed)
			{
				data.SequentialView->set_value(data.SequentialIndex, value);
				AddSequentialItemButtons(ss, data, insertValue);
			}
			// Call the custom set-value function if provided, usually for properties of custom struct/class inside a sequential container
			else if (func)
			{
				func(data);
			}
		}
		// Switch to next line's left column
		ImGui::NextColumn();
	}

	void EditorLayer::EndDisplayProperty(std::stringstream& ss, const PropertyData& data, rttr::argument insertValue)
	{
		ShowPropertyTooltip(*data.Property);
		if (data.SequentialIndex != -1 && !data.bPropertyRecursed)
		{
			AddSequentialItemButtons(ss, data, insertValue);
		}
		// Switch to next line's left column
		ImGui::NextColumn();
	}

	void EditorLayer::ProcessBoolType(bool boolValue, const PropertyData& data)
	{
		std::stringstream ss;
		BeginDisplayProperty(ss, data);
		ImGui::Checkbox(ss.str().c_str(), &boolValue);
		EndDisplayProperty(ss, data, boolValue, false);
	}

	void EditorLayer::ProcessEnumType(const PropertyData& data)
	{
		std::stringstream ss;
		BeginDisplayProperty(ss, data);
		bool bOK = false;
		std::string stringValue = data.PropertyValue->to_string(&bOK);
		if (!bOK)
		{
			LogPropertyMessage(*data.Property, "Failed to convert an enum value of property {0} to string!", 3);
		}
		rttr::enumeration enumType = data.PropertyValue->get_type().get_enumeration();
		auto& enumValues = enumType.get_values();
		if (ImGui::BeginCombo(ss.str().c_str(), stringValue.c_str()))
		{
			for (const auto& enumValue : enumValues)
			{
				if (ImGui::Selectable(enumType.value_to_name(enumValue).data()))
				{
					SetPropertyValue(data, enumValue, enumValue, [&enumValue](const PropertyData& data) {
						rttr::variant& sequentialVar = data.SequentialView->get_value(data.SequentialIndex);
						if (sequentialVar.can_convert<BurstData>())
						{
							sequentialVar.get_value<BurstData*>()->Amount.VariationType = enumValue.get_value<ParticleVariationType>();
						}
					});
					// Selection changes, call the callback
					if (enumValue != enumType.name_to_value(stringValue))
					{
						InvokePropertyChangeCallback(data);
					}
				}
			}
			ImGui::EndCombo();
		}
		EndDisplayProperty(ss, data, *enumValues.cbegin());
	}

	void EditorLayer::ProcessStringType(std::string* stringPointerValue, const PropertyData& data)
	{
		std::stringstream ss;
		BeginDisplayProperty(ss, data);
		// Map from id to temp edited string plus a bool flag indicating if we are editing the InputText
		static std::unordered_map<std::string, std::pair<bool, std::string>> valueBuffer;
		ImGui::InputText(ss.str().c_str(), valueBuffer[ss.str()].first ? &valueBuffer[ss.str()].second : stringPointerValue, ImGuiInputTextFlags_AutoSelectAll);
		// Write changes to cache first
		if (ImGui::IsItemActivated())
		{
			valueBuffer[ss.str()].first = true;
			valueBuffer[ss.str()].second = *stringPointerValue;
		}
		bool bChanged = false;
		// Apply cache when user finishes editing
		if (ImGui::IsItemDeactivated())
		{
			// "Name" property specific
			if (data.Property->get_name() == "Name")
			{
				Level::Get().m_ObjectNames.erase(*stringPointerValue);
				Level::Get().m_ObjectNames.emplace(valueBuffer[ss.str()].second);
			}
			bChanged = valueBuffer[ss.str()].second != *stringPointerValue;
			*stringPointerValue = std::move(valueBuffer[ss.str()].second);
			valueBuffer[ss.str()].first = false;
		}
		EndDisplayProperty(ss, data, *stringPointerValue, std::string(""));
		if (bChanged)
		{
			InvokePropertyChangeCallback(data);
		}
	}

	void EditorLayer::ProcessVec2Type(glm::vec2* vec2PointerValue, const PropertyData& data)
	{
		std::stringstream ss;
		BeginDisplayProperty(ss, data);
		static std::unordered_map<std::string, glm::vec2> valueBuffer;
		valueBuffer[ss.str()] = *vec2PointerValue;
		rttr::variant speedVar = data.Property->get_metadata(PropertyMeta::DragSensitivity);
		rttr::variant minVar = data.Property->get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = data.Property->get_metadata(PropertyMeta::Max);
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		float min = minVar ? std::max(minVar.to_float(), -FLT_MAX) : -FLT_MAX;
		float max = maxVar ? std::min(maxVar.to_float(), FLT_MAX) : FLT_MAX;
		bool bIsDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
		bool bChanged = ImGui::DragFloat2(ss.str().c_str(), bIsDragging ? glm::value_ptr(*vec2PointerValue) : glm::value_ptr(valueBuffer[ss.str()]), speed, min, max, "%.2f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			*vec2PointerValue = std::move(glm::clamp(valueBuffer[ss.str()], min, max));
			EndDisplayProperty(ss, data, *vec2PointerValue, glm::vec2(0.0f));
			InvokePropertyChangeCallback(data);
		}
		else
		{
			if (ImGui::IsKeyPressed(ZE_KEY_TAB) && bChanged)
			{
				*vec2PointerValue = std::move(glm::clamp(valueBuffer[ss.str()], min, max));
				EndDisplayProperty(ss, data, *vec2PointerValue, glm::vec2(0.0f));
				InvokePropertyChangeCallback(data);
			}
			else
			{
				*vec2PointerValue = glm::clamp(*vec2PointerValue, min, max);
				EndDisplayProperty(ss, data, *vec2PointerValue, glm::vec2(0.0f));
			}
		}
		if (bChanged && bIsDragging)
		{
			InvokePropertyChangeCallback(data, false);
		}
	}

	void EditorLayer::ProcessI32Vec2Type(glm::i32vec2* i32vec2PointerValue, const PropertyData& data)
	{
		std::stringstream ss;
		BeginDisplayProperty(ss, data);
		static std::unordered_map<std::string, glm::i32vec2> valueBuffer;
		valueBuffer[ss.str()] = *i32vec2PointerValue;
		rttr::variant speedVar = data.Property->get_metadata(PropertyMeta::DragSensitivity);
		rttr::variant minVar = data.Property->get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = data.Property->get_metadata(PropertyMeta::Max);
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		int32_t min = minVar ? std::max(minVar.to_int32(), INT32_MIN) : INT32_MIN;
		int32_t max = maxVar ? std::min(maxVar.to_int32(), INT32_MAX) : INT32_MAX;
		bool bIsDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
		bool bChanged = ImGui::DragInt2(ss.str().c_str(), bIsDragging ? glm::value_ptr(*i32vec2PointerValue) : glm::value_ptr(valueBuffer[ss.str()]), speed, min, max);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			*i32vec2PointerValue = std::move(glm::clamp(valueBuffer[ss.str()], min, max));
			EndDisplayProperty(ss, data, *i32vec2PointerValue, glm::i32vec2(0));
			InvokePropertyChangeCallback(data);
		}
		else
		{
			if (ImGui::IsKeyPressed(ZE_KEY_TAB) && bChanged)
			{
				*i32vec2PointerValue = std::move(glm::clamp(valueBuffer[ss.str()], min, max));
				EndDisplayProperty(ss, data, *i32vec2PointerValue, glm::i32vec2(0));
				InvokePropertyChangeCallback(data);
			}
			else
			{
				*i32vec2PointerValue = glm::clamp(*i32vec2PointerValue, min, max);
				EndDisplayProperty(ss, data, *i32vec2PointerValue, glm::i32vec2(0));
			}
		}
		if (bChanged && bIsDragging)
		{
			InvokePropertyChangeCallback(data, false);
		}
	}

	void EditorLayer::ProcessVec3Type(glm::vec3* vec3PointerValue, const PropertyData& data)
	{
		std::stringstream ss;
		BeginDisplayProperty(ss, data);
		static std::unordered_map<std::string, glm::vec3> valueBuffer;
		valueBuffer[ss.str()] = *vec3PointerValue;
		rttr::variant speedVar = data.Property->get_metadata(PropertyMeta::DragSensitivity);
		rttr::variant minVar = data.Property->get_metadata(PropertyMeta::Min);
		rttr::variant maxVar = data.Property->get_metadata(PropertyMeta::Max);
		float speed = speedVar ? speedVar.to_float() : 1.0f;
		float min = minVar ? std::max(minVar.to_float(), -FLT_MAX) : -FLT_MAX;
		float max = maxVar ? std::min(maxVar.to_float(), FLT_MAX) : FLT_MAX;
		bool bIsDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
		bool bChanged = ImGui::DragFloat3(ss.str().c_str(), bIsDragging ? glm::value_ptr(*vec3PointerValue) : glm::value_ptr(valueBuffer[ss.str()]), speed, min, max, "%.2f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			*vec3PointerValue = std::move(glm::clamp(valueBuffer[ss.str()], min, max));
			EndDisplayProperty(ss, data, *vec3PointerValue, glm::vec3(0.0f));
			InvokePropertyChangeCallback(data);
		}
		else
		{
			if (ImGui::IsKeyPressed(ZE_KEY_TAB) && bChanged)
			{
				*vec3PointerValue = std::move(glm::clamp(valueBuffer[ss.str()], min, max));
				EndDisplayProperty(ss, data, *vec3PointerValue, glm::vec3(0.0f));
				InvokePropertyChangeCallback(data);
			}
			else
			{
				*vec3PointerValue = glm::clamp(*vec3PointerValue, min, max);
				EndDisplayProperty(ss, data, *vec3PointerValue, glm::vec3(0.0f));
			}
		}
		if (bChanged && bIsDragging)
		{
			InvokePropertyChangeCallback(data, false);
		}
	}

	void EditorLayer::ProcessColorType(glm::vec4* vec4PointerValue, const PropertyData& data)
	{
		std::stringstream ss;
		BeginDisplayProperty(ss, data);
		static std::unordered_map<std::string, glm::vec4> valueBuffer;
		valueBuffer[ss.str()] = *vec4PointerValue;
		bool bIsDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
		bool bChanged = ImGui::ColorEdit4(ss.str().c_str(), bIsDragging ? glm::value_ptr(*vec4PointerValue) : glm::value_ptr(valueBuffer[ss.str()]));
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			*vec4PointerValue = std::move(valueBuffer[ss.str()]);
			EndDisplayProperty(ss, data, *vec4PointerValue, glm::vec4(0.0f));
			InvokePropertyChangeCallback(data);
		}
		else
		{
			if (ImGui::IsKeyPressed(ZE_KEY_TAB) && bChanged)
			{
				*vec4PointerValue = std::move(valueBuffer[ss.str()]);
				EndDisplayProperty(ss, data, *vec4PointerValue, glm::vec4(0.0f));
				InvokePropertyChangeCallback(data);
			}
			else
			{
				EndDisplayProperty(ss, data, *vec4PointerValue, glm::vec4(0.0f));
			}
		}
		if (bChanged && bIsDragging)
		{
			InvokePropertyChangeCallback(data, false);
		}
	}

	void EditorLayer::ProcessGameObjectType(GameObject* gameObjectValue, const PropertyData& data)
	{
		std::stringstream ss;
		BeginDisplayProperty(ss, data);
		std::stringstream ss_PreviewName;
		if (gameObjectValue && !gameObjectValue->IsPendingDestroy())
		{
			// Displayed as "ObjectName (ClassName)"
			ss_PreviewName << gameObjectValue->GetName() << " (" << rttr::type::get(*gameObjectValue).get_name() << ")";
		}
		// If current chosen GameObject is deleted from level, zero the pointer
		else
		{
			SetPropertyValue(data, nullptr, static_cast<GameObject*>(nullptr));
		}
		if (ImGui::BeginCombo(ss.str().c_str(), ss_PreviewName.str().c_str()))
		{
			// A specialized entry for clearing out current selection (empty the pointer)
			if (ImGui::Selectable("Null"))
			{
				SetPropertyValue(data, nullptr, static_cast<GameObject*>(nullptr));
			}

			auto& level = Level::Get();
			for (uint32_t i = 0; i < level.m_GameObjects.size(); ++i)
			{
				if (level.m_GameObjects[i]->IsPendingDestroy() || !IsSubclassOf(level.m_GameObjects[i], *data.Property))
					continue;

				std::stringstream ss_ObjectName;
				ss_ObjectName << level.m_GameObjects[i]->GetName() << " (" << rttr::type::get(*level.m_GameObjects[i]).get_name() << ")";
				if (ImGui::Selectable(ss_ObjectName.str().c_str()))
				{
					// Note: The pointer assignment will only succeed if assigned-from pointer type is exactly the same as assigned-to pointer type
					// That's to say, GameObject* cannot be assigned to Player* even if that GameObject* object is indeed a Player* object
					SetPropertyValue(data, level.m_GameObjects[i], level.m_GameObjects[i]);
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
				GameObject* draggedGameObject = *static_cast<GameObject**>(payload->Data);
				if (IsSubclassOf(draggedGameObject, *data.Property))
				{
					SetPropertyValue(data, draggedGameObject, draggedGameObject);
				}
			}
			ImGui::EndDragDropTarget();
		}
		EndDisplayProperty(ss, data, static_cast<GameObject*>(nullptr));
	}

	void EditorLayer::ProcessParticleSystemType(ParticleSystem* particleSystemValue, const PropertyData& data)
	{
		std::stringstream ss;
		BeginDisplayProperty(ss, data);
		// TODO: Add search filter for particle system
		if (ImGui::BeginCombo(ss.str().c_str(), particleSystemValue ? particleSystemValue->GetFileName().c_str() : nullptr))
		{
			ParticleLibrary* library = GetParticleLibrary();
			// Pop up file browser to select a particle system
			if (ImGui::Selectable("Browse particle system..."))
			{
				nfdchar_t* outPath = nullptr;
				nfdresult_t result = NFD_OpenDialog("zparticle", nullptr, &outPath);
				if (result == NFD_OKAY)
				{
					const std::string relativePath = ToRelativePath(outPath);
					// Add selected particle system to the library
					ParticleSystem* loadedPs = library->GetOrLoad(relativePath);
					SetPropertyValue(data, loadedPs, loadedPs);
					free(outPath);
				}
				else if (result == NFD_ERROR)
				{
					ZE_CORE_ERROR("ProcessParticleSystemType: {0}", NFD_GetError());
				}
			}
			ImGui::Separator();
			// List all loaded particle systems from ParticleLibrary
			for (const auto& [path, ps] : library->GetParticlesMap())
			{
				if (ImGui::Selectable(ps->GetFileName().c_str()))
				{
					SetPropertyValue(data, ps, ps);
				}
				// Display particle system path tooltip
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", ps->GetPath().c_str());
				}
			}
			ImGui::EndCombo();
		}
		if (particleSystemValue)
		{
			// Display particle system path tooltip
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(u8"%s\n(Ë«»÷Êó±êÓÒ¼ü´ò¿ªÁ£×Ó±à¼­Æ÷)", particleSystemValue->GetPath().c_str());
				// Open Particle Editor on right mouse button double clicked
				// Note: IsItemHovered() must be true at the same time or multiple combo boxes will be affected
				if (ImGui::IsMouseDoubleClicked(1))
				{
					m_bShowParticleEditor = true;
					LoadParticleSystemFromFile(particleSystemValue->GetPath().c_str());
					m_CurrentParticleSystemPath = ToAbsolutePath(particleSystemValue->GetPath().c_str());
					m_CurrentParticleSystemName = particleSystemValue->GetFileName();
				}
			}
		}
		EndDisplayProperty(ss, data, static_cast<ParticleSystem*>(nullptr));
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

	std::string EditorLayer::ToRelativePath(const char* absolutePath)
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

	std::string EditorLayer::ToAbsolutePath(const char* relativePath)
	{
		return std::filesystem::current_path().string() + "/" + relativePath;
	}

}
