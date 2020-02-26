#pragma once

#include "Engine/Core/Layer.h"

#include <glm/glm.hpp>
#include <rttr/type>

#include "Engine/Renderer/OrthographicCameraController.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/ParticleSystem.h"

namespace ZeoEngine {

	class GameObject;

	enum class PIEState
	{
		None,
		Running,
		Paused
	};

	extern PIEState g_PIEState;

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

		const Scope<OrthographicCameraController>& GetGameViewCameraController() const { return m_GameViewCameraController; }
		OrthographicCamera* GetGameViewCamera() const { return m_GameViewCameraController ? &m_GameViewCameraController->GetCamera() : nullptr; }
		// TODO: ClearSelectedGameObject()
		void ClearSelectedGameObject() { m_SelectedGameObject = nullptr; }

	private:
		void LoadEditorTextures();

		void CreateMainEditorDockspace();
		void ShowGameView(bool* bShow);
		void ShowLevelOutline(bool* bShow);
		void ShowObjectInspector(bool* bShow);
		void ShowClassBrowser(bool* bShow);
		void ShowConsole(bool* bShow);

		void CreateParticleEditorDockspace(bool* bShow);
		void ShowParticleEditor(bool* bShow);

		void ShowPreferences(bool* bShow);
		void ShowAbout(bool* bShow);

		void SetNextWindowDefaultPosition();

		bool OnKeyPressed(KeyPressedEvent& e);
		void OnGameViewWindowResized(float newSizeX, float newSizeY);
		// TODO: You should check to call this every time you add a new object selection method
		void OnGameObjectSelectionChanged(GameObject* lastSelectedGameObject);

		/** Show transform options and draw transform gizmo. */
		void EditTransform();

		void CreateDefaultParticleSystem();
		void LoadParticleSystemFromFile(const char* particleSystemPath);
		void SaveParticleSystemToFile(std::string& particleSystemPath);

		void StartPIE();
		void StopPIE();
		void PausePIE();
		void ResumePIE();

		void ConstructClassInheritanceTree();
		void ConstructClassInheritanceTreeRecursively(const rttr::type& baseType, const rttr::array_range<rttr::type>& derivedTypes);
		void DisplayClassHierarchyRecursively(const std::vector<rttr::type>& derivedTypes);
		void ProcessClassInteraction(const rttr::type& type);

#define BEGIN_PROP(prop) \
		ImGui::Columns(2);\
		ImGui::AlignTextToFramePadding();\
		ImGui::TreeNodeEx(prop->get_name().data(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet); /*Property name*/\
		ShowPropertyTooltip(*prop);\
		ImGui::NextColumn(); /*Switch to the right column*/\
		ImGui::SetNextItemWidth(-1.0f); /*Align width to the right side*/\
		ss << "##" << prop->get_name().data(); /*We use property name as id here because rttr guarantees that only properties with different names can be registered*/
#define END_PROP(prop) \
		ShowPropertyTooltip(*prop);\
		ImGui::NextColumn(); /*Switch to next line's left column*/
#define END_SETPROP(prop, instance, value) \
		END_PROP(prop)\
		prop->set_value(*instance, value);
#define BEGIN_SEQPROP(prop, sequentialIndex, bPropRecursed) \
		ImGui::Columns(2);\
		ImGui::AlignTextToFramePadding();\
		if (bPropRecursed)\
		{\
			ss << prop->get_name().data(); /**Property of custom struct/class inside a sequential container*/\
		}\
		else\
		{\
			ss << "[" << sequentialIndex << "]"; /**Property inside a sequential container*/\
		}\
		ImGui::TreeNodeEx(ss.str().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet); /*Sequential index*/\
		ShowPropertyTooltip(*prop);\
		ImGui::NextColumn(); /*Switch to the right column*/\
		ss.clear(); ss.str("");\
		ss << "##" << prop->get_name().data() << sequentialIndex;
#define ADD_SEQBUTTONS(prop, sequentialView, sequentialIndex, insertValue) \
		ImGui::SameLine();\
		ss.clear(); ss.str(""); ss << "##" << prop->get_name() << sequentialView->get_value_type().get_name() << sequentialIndex;\
		if (ImGui::BeginCombo(ss.str().c_str(), nullptr, ImGuiComboFlags_NoPreview))\
		{\
			ss.clear(); ss.str(""); ss << "Insert##" << prop->get_name() << sequentialView->get_value_type().get_name() << sequentialIndex;\
			if (ImGui::Selectable(ss.str().c_str()))\
			{\
				sequentialView->insert(sequentialView->begin() + sequentialIndex, insertValue);\
			}\
			ss.clear(); ss.str(""); ss << "Erase##" << prop->get_name() << sequentialView->get_value_type().get_name() << sequentialIndex;\
			if (ImGui::Selectable(ss.str().c_str()))\
			{\
				sequentialView->erase(sequentialView->begin() + sequentialIndex);\
			}\
			ImGui::EndCombo();\
		}
#define END_SETSEQPROP(prop, sequentialView, sequentialIndex, value, insertValue) \
		ShowPropertyTooltip(*prop);\
		sequentialView->set_value(sequentialIndex, value);\
		ADD_SEQBUTTONS(prop, sequentialView, sequentialIndex, insertValue)\
		ImGui::NextColumn();

		/** Show tooltip on top of classes or structs if available. */
		void ShowTypeTooltip(const rttr::type& type);
		/**
		 * Show tooltip on top of every outermost property and inner property if available.
		 * You should put this method after the line constructing the widget that you want to show tooltip.
		 */
		void ShowPropertyTooltip(const rttr::property& prop);

		struct PropertyData
		{
			/** False if current property is the outermost property (GameObject::m_Struct rather than GameObject::m_Struct.property) */
			bool bPropertyRecursed = false;
			rttr::property* Property = nullptr, *OuterProperty = nullptr;
			rttr::instance* Object = nullptr, *OutermostObject = nullptr;
			rttr::variant* PropertyValue = nullptr;
			rttr::variant_sequential_view* SequentialView = nullptr;
			rttr::variant_associative_view* AssociativeView = nullptr;
			int32_t SequentialIndex = -1;
		};

		/** Categorizing properties before displaying them. */
		void PreProcessProperties(const rttr::instance& object);
		void ProcessPropertiesRecursively(PropertyData& data);
		void ProcessProperty(PropertyData& data);
		bool ProcessPropertyValue(PropertyData& data);
		bool ProcessAtomicTypes(const rttr::type& type, PropertyData& data);
		void AddSequentialButtons(const PropertyData& data);
		void ProcessSequentialContainerTypes(PropertyData& data);
		void ProcessAssociativeContainerTypes(PropertyData& data);

		/** logLevel - 0: trace, 1: info, 2: warning, 3: error, 4: critical */
		void LogPropertyMessage(const rttr::property& prop, const char* msg, uint32_t logLevel);

		void InvokePropertyChangeCallback(const PropertyData& data);
		
		// Every copied property type must take an object as a parameter
		void ProcessBoolType(bool boolValue,  const PropertyData& data);
		void ProcessInt8Type(int8_t int8Value, const PropertyData& data);
		void ProcessInt32Type(int32_t int32Value, const PropertyData& data);
		void ProcessInt64Type(int64_t int64Value, const PropertyData& data);
		void ProcessUInt8Type(uint8_t uint8Value, const PropertyData& data);
		void ProcessUInt32Type(uint32_t uint32Value, const PropertyData& data);
		void ProcessUInt64Type(uint64_t uint64Value, const PropertyData& data);
		void ProcessFloatType(float floatValue, const PropertyData& data);
		void ProcessDoubleType(double doubleValue, const PropertyData& data);
		void ProcessEnumType(const PropertyData& data);
		void ProcessStringType(std::string* stringPointerValue, const PropertyData& data);
		void ProcessVec2Type(glm::vec2* vec2PointerValue, const PropertyData& data);
		void ProcessI32Vec2Type(glm::i32vec2* i32vec2PointerValue, const PropertyData& data);
		void ProcessVec3Type(glm::vec3* vec3PointerValue, const PropertyData& data);
		void ProcessColorType(glm::vec4* vec4PointerValue, const PropertyData& data);
		void ProcessGameObjectType(GameObject* gameObjectValue, const PropertyData& data);
		void ProcessTexture2DType(const Ref<Texture2D>& texture2DValue, const PropertyData& data);
		void ProcessParticleSystemType(ParticleSystem* particleSystemValue, const PropertyData& data);
		/**
		 * Returns true if gameObject's class is derived from the class specified by PropertyMeta::SubclassOf
		 * or PropertyMeta::SubclassOf is not specified at all.
		 */
		bool IsSubclassOf(GameObject* gameObject, const rttr::property& prop);

		// TODO: Maybe move it to dedicated utilities class in the future
		/** Convert absolute path to relative path. */
		std::string ToRelativePath(const char* absolutePath);
		/** Convert relative path to absolute path. */
		std::string ToAbsolutePath(const char* relativePath);

	private:
		Scope<OrthographicCameraController> m_GameViewCameraController, m_ParticleViewCameraController;
		Ref<Texture2D> m_PlayTexture, m_PauseTexture, m_StopTexture, m_ToolBarTextures[2],
			m_LogoTexture;

		bool m_bIsHoveringGameView = false;
		glm::vec2 m_LastGameViewSize;
		bool m_bResetLayout = false;

		std::string m_CurrentLevelPath, m_CurrentLevelName;

		bool m_bShowParticleEditor = false;
		std::string m_CurrentParticleSystemPath, m_CurrentParticleSystemName;
		ParticleSystem* m_EditorParticleSystem = nullptr;
		bool m_bIsHoveringParticleView = false;
		glm::vec2 m_LastParticleViewSize;

		/** Map from parent class type to its child class types */
		std::unordered_map<rttr::type, std::vector<rttr::type>> m_ClassInheritanceTree;
		/** Stores a list of class types which have already been handled to prevent them from being handled again */
		std::set<rttr::type> m_DisplayedClasses;

		GameObject* m_SelectedGameObject = nullptr;

		enum PropertySource
		{
			GameObjectProperty,
			ParticleSystemProperty
		}m_CurrentPropertySource;
#define PROPERTY_SOURCE_NUM 2
		/** Map from property category name to properties of that category, if category name is not specified, name "default" will be used */
		std::map<std::string, std::vector<rttr::property>> m_SortedProperties[PROPERTY_SOURCE_NUM];
		/** Flag used to prevent sorting properties every frame, it will only sort them when current object is changed */
		bool m_bIsSortedPropertiesDirty[PROPERTY_SOURCE_NUM]{ true, true };
		/**
		 * Used to prevent logging identical messages every frame.
		 * Although we can share this across different property sources, we want to improve performance by reserving capacity beforehand, which is not possible for sharing case
		 */
		std::vector<rttr::property> m_PropertiesLogged[PROPERTY_SOURCE_NUM];

		/** Flag used to prevent calling RecomposeTransformMatrix() every frame */
		bool m_bIsTransformDirty = false;

		/**
		 * Map from InputText id to temp edited string plus a bool flag indicating if we are editing that InputText.
		 * This variable is necessary as InputText() requires a persistent string to read and write, and we only want the input changes to apply when user presses enter etc - deferred editing.
		 */
		std::unordered_map<std::string, std::pair<bool, std::string>> m_TempInputStrings;

	};

}
