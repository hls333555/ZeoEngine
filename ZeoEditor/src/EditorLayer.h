#pragma once

#include "Engine/GameFramework/EngineLayer.h"

#include <glm/glm.hpp>
#include <rttr/type>

#include "Engine/Events/KeyEvent.h"
#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	class EditorDockspace;

	struct RTTRPropertyHashHasher
	{
		size_t operator()(const rttr::property& prop) const
		{
			return std::hash<std::string>{}(prop.get_name().to_string());
		}
	};

	enum class PIEState
	{
		None,
		Running,
		Paused
	};

	extern PIEState pieState;

	constexpr uint32_t propertySourceNum = 2;

	class EditorLayer : public EngineLayer
	{
		struct PropertyData
		{
			/** False if current property is the outermost property (GameObject::m_Struct rather than GameObject::m_Struct.property) */
			bool bPropertyRecursed = false;
			rttr::property* Property = nullptr, *OuterProperty = nullptr;
			rttr::instance* Object = nullptr, *OutermostObject = nullptr;
			rttr::variant* PropertyValue = nullptr;
			rttr::variant_sequential_view* SequentialView = nullptr;
			rttr::variant_associative_view* AssociativeView = nullptr;
			/** -1 means it is not an item in a sequential container */
			int32_t SequentialIndex = -1;
		};

		using RecursedSequentialPropertyProcessingFn = std::function<void(const PropertyData&)>;

	public:
		EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(DeltaTime dt) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

		DockspaceManager& GetDockspaceManager() { return m_DockspaceManager; }

		void PushDockspace(EditorDockspace* dockspace);

		//virtual OrthographicCamera* GetGameCamera() override { return &m_CameraControllers[GAME_VIEW_PIE]->GetCamera(); }

		void ClearSelectedGameObject() { m_SelectedGameObject = nullptr; }

	private:
		void ShowLevelOutline(bool* bShow);
		void ShowObjectInspector(bool* bShow);
		void ShowClassBrowser(bool* bShow);

		void CreateParticleEditorDockspace(bool* bShow);
		void ShowParticleEditor(bool* bShow);

		void SetNextWindowDefaultPosition();

		bool OnKeyPressed(KeyPressedEvent& e);

		// TODO: You should check to call this every time you add a new object selection method
		void OnGameObjectSelectionChanged(GameObject* lastSelectedGameObject);
		void OnGameViewImGuiRender();
		/** Show transform options and draw transform gizmo. */
		void EditTransform();
		void DrawCollision();

		void LoadParticleSystemFromFile(const char* particleSystemPath);
		void SaveParticleSystemToFile(std::string& particleSystemPath);

		void StartPIE();
		void StopPIE();
		void PausePIE();
		void ResumePIE();

		void DisplayClassHierarchyRecursively(const std::vector<rttr::type>& derivedTypes);
		void ProcessClassInteraction(const rttr::type& type);

		/** Show tooltip on top of classes or structs if available. */
		void ShowTypeTooltip(const rttr::type& type);
		/**
		 * Show tooltip on top of every outermost property and inner property if available.
		 * You should put this method after the line constructing the widget that you want to show tooltip.
		 */
		void ShowPropertyTooltip(const rttr::property& prop);

		/** Categorizing properties before displaying them. */
		void PreProcessProperties(rttr::instance object);
		void ProcessPropertiesRecursively(PropertyData& data);
		void ProcessProperty(PropertyData& data);
		bool ProcessPropertyValue(PropertyData& data);
		bool ProcessAtomicTypes(const rttr::type& type, PropertyData& data);
		void AddSequentialContainerButtons(const PropertyData& data);
		void ProcessSequentialContainerTypes(PropertyData& data);
		void ProcessAssociativeContainerTypes(PropertyData& data);

		/** logLevel - 0: trace, 1: info, 2: warning, 3: error, 4: critical */
		void LogPropertyMessage(const rttr::property& prop, const char* msg, uint32_t logLevel);

		/** Invoke a callback function to reflect property value changes. Should be called after property value is updated. */
		void InvokePropertyChangeCallback(const PropertyData& data, bool bInvokeOnlyIfDeactivated = true);

		// Utility methods for displaying various properties via ImGui
		void BeginDisplayProperty(std::stringstream& ss, const PropertyData& data);
		void AddSequentialItemButtons(std::stringstream& ss, const PropertyData& data, rttr::argument insertValue);
		void SetPropertyValue(const PropertyData& data, rttr::argument value, rttr::argument insertValue, RecursedSequentialPropertyProcessingFn func = nullptr);
		void EndDisplayProperty(std::stringstream& ss, const PropertyData& data, rttr::argument value, rttr::argument insertValue, RecursedSequentialPropertyProcessingFn func = nullptr);
		void EndDisplayProperty(std::stringstream& ss, const PropertyData& data, rttr::argument insertValue);
		
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
		DockspaceManager m_DockspaceManager;

		std::string m_CurrentLevelPath, m_CurrentLevelName;

		bool m_bShowParticleEditor = false;
		std::string m_CurrentParticleSystemPath, m_CurrentParticleSystemName;
		ParticleSystem* m_EditorParticleSystem = nullptr;
		glm::vec2 m_LastParticleViewSize;

		/** Map from parent class type to its child class types */
		std::unordered_map<rttr::type, std::vector<rttr::type>> m_ClassInheritanceTree;
		/** Stores a list of class types which have already been handled to prevent them from being handled again */
		std::set<rttr::type> m_DisplayedClasses;

		GameObject* m_SelectedGameObject = nullptr;

		enum PropertySource
		{
			GAMEOBJECT_PROP = 0,
			PARTICLESYSTEM_PROP = 1
		} m_CurrentPropertySource;
		/** Map from property category name to properties of that category, if category name is not specified, name "default" will be used */
		std::map<std::string, std::vector<rttr::property>> m_SortedProperties[propertySourceNum];
		/** Flag used to prevent sorting properties every frame, it will only sort them when current object is changed */
		bool m_bIsSortedPropertiesDirty[propertySourceNum]{ true, true };
		/**
		 * Used to prevent logging identical messages every frame.
		 * Although we can share this across different property sources, we want to improve performance by reserving capacity beforehand, which is not possible for sharing case
		 */
		std::unordered_set<rttr::property, RTTRPropertyHashHasher> m_LoggedProperties[propertySourceNum];
		/**
		 * Map from property to {condition property, condition property value} pair.
		 * Used to prevent processing HideCondition properties every frame.
		 * Although we can share this across different property sources, we want to improve performance by reserving capacity beforehand, which is not possible for sharing case
		 */
		std::unordered_map<rttr::property, std::pair<rttr::property, std::string>, RTTRPropertyHashHasher> m_HideConditionProperties[propertySourceNum];

	};

}
