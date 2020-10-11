#include "Panels/EntityInspectorPanel.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/GameFramework/Components.h"
#include "Dockspaces/MainDockspace.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

	void EntityInspectorPanel::RenderPanel()
	{
		Entity selectedEntity = GetContext<MainDockspace>()->m_SelectedEntity;
		if (selectedEntity != m_LastSelectedEntity && m_LastSelectedEntity)
		{
			// Sometimes, selected entity is changed when certain input box is still active, ImGui::IsItemDeactivatedAfterEdit() of that item will not get called,
			// so we have to draw last entity's components once again to ensure all caches are applied
			DrawComponents(m_LastSelectedEntity);
			m_LastSelectedEntity = selectedEntity;
			return;
		}
		if (selectedEntity)
		{
			DrawComponents(selectedEntity);
		}
		m_LastSelectedEntity = selectedEntity;
	}

	void EntityInspectorPanel::DrawComponents(Entity entity)
	{
		// Push entity id for later use
		ImGui::PushID(static_cast<uint32_t>(entity));

		// We want to draw these components first as the iteration order is backward
		if (entity.HasComponent<TagComponent>())
		{
			ProcessType(entt::resolve<TagComponent>(), entity);
		}
		if (entity.HasComponent<TransformComponent>())
		{
			ProcessType(entt::resolve<TransformComponent>(), entity);

			//if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			//{
			//	auto& transform = entity.GetComponent<TransformComponent>().Transform;

			//	ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);

			//	ImGui::TreePop();
			//}
		}

		GetScene()->m_Registry.visit(entity, [this, entity](const auto componentId)
		{
			const auto type = entt::resolve_type(componentId);
			if (IsTypeEqual<TagComponent>(type) || IsTypeEqual<TransformComponent>(type)) return;

			ProcessType(type, entity);
		});

		ImGui::PopID();
	}

	uint32_t EntityInspectorPanel::GetUniqueDataID(entt::meta_data data)
	{
		return ImGui::GetCurrentWindow()->GetID(data.id());
	}

	bool EntityInspectorPanel::ShouldHideData(entt::meta_data data, entt::meta_any instance)
	{
		auto hideCondition = GetPropValue<const char*>(PropertyType::HideCondition, data);
		// HideCondition property is not set, show this data normally
		if (!hideCondition) return false;

		// Map from id to HideCondition key-value pair
		static std::unordered_map<uint32_t, std::pair<std::string, std::string>> hideConditionBuffers;
		auto id = GetUniqueDataID(data);

		// TODO: Add more operators
		std::string hideConditionStr{ *hideCondition };
		auto tokenPos = hideConditionStr.find("!=");
		if (tokenPos != std::string::npos)
		{
			std::string keyStr, valueStr;
			if (hideConditionBuffers.find(id) != hideConditionBuffers.end())
			{
				keyStr = hideConditionBuffers[id].first;
				valueStr = hideConditionBuffers[id].second;
			}
			else
			{
				keyStr = hideConditionStr.substr(0, tokenPos);
				// Erase tail blanks
				keyStr.erase(keyStr.find_last_not_of(" ") + 1);

				valueStr = hideConditionStr.substr(tokenPos + 2, hideConditionStr.size() - 1);
				// Erase head blanks
				valueStr.erase(0, valueStr.find_first_not_of(" "));
				// Extract enum value (e.g. SceneCamera::ProjectionType::Perspective -> Perspective)
				auto valuePos = valueStr.rfind("::");
				if (valuePos != std::string::npos)
				{
					valueStr.erase(0, valuePos + 2);
				}

				hideConditionBuffers[id].first = keyStr;
				hideConditionBuffers[id].second = valueStr;
			}

			auto keyData = entt::resolve_type(instance.type().type_id()).data(entt::hashed_string{ keyStr.c_str() });
			auto keyDataValue = keyData.get(instance);
			auto keyDataType = keyData.type();
			auto valueToCompare = keyDataType.data(entt::hashed_string{ valueStr.c_str() }).get({});
			return keyDataValue != valueToCompare;
		}

		return false;
	}

	void EntityInspectorPanel::ProcessType(entt::meta_type type, Entity entity)
	{
		const auto instance = GetTypeInstance(type, GetScene()->m_Registry, entity);
		auto name = GetPropValue<const char*>(PropertyType::Name, type);

		bool bIsExpanded = ImGui::CollapsingHeader(*name, ImGuiTreeNodeFlags_DefaultOpen);
		ShowPropertyTooltip(type);
		if (bIsExpanded)
		{
			type.data([this, instance](entt::meta_data data)
			{
				if (ShouldHideData(data, instance)) return;

				if (data.type().is_integral())
				{
					ProcessIntegralData(data, instance);
				}
				else if (data.type().is_floating_point())
				{
					ProcessFloatingPointData(data, instance);
				}
				else if (data.type().is_enum())
				{
					ProcessEnumData(data, instance);
				}
				else
				{
					ProcessOtherData(data, instance);
				}
			});
		}
	}

	void EntityInspectorPanel::ProcessIntegralData(entt::meta_data data, entt::meta_any instance)
	{
		if (IsTypeEqual<bool>(data.type()))
		{
			ProcessBoolData(data, instance);
		}
		else if (IsTypeEqual<int8_t>(data.type()))
		{
			ProcessScalarNData<int8_t>(data, instance, ImGuiDataType_S8, static_cast<int8_t>(INT8_MIN), static_cast<int8_t>(INT8_MAX));
		}
		else if (IsTypeEqual<int32_t>(data.type()))
		{
			ProcessScalarNData<int32_t>(data, instance, ImGuiDataType_S32, INT32_MIN, INT32_MAX);
		}
		else if (IsTypeEqual<int64_t>(data.type()))
		{
			ProcessScalarNData<int64_t>(data, instance, ImGuiDataType_S64, INT64_MIN, INT64_MAX);
		}
		else if (IsTypeEqual<uint8_t>(data.type()))
		{
			ProcessScalarNData<uint8_t>(data, instance, ImGuiDataType_U8, 0ui8, UINT8_MAX);
		}
		else if (IsTypeEqual<uint32_t>(data.type()))
		{
			ProcessScalarNData<uint32_t>(data, instance, ImGuiDataType_U32, 0ui32, UINT32_MAX);
		}
		else if (IsTypeEqual<uint64_t>(data.type()))
		{
			ProcessScalarNData<uint64_t>(data, instance, ImGuiDataType_U64, 0ui64, UINT64_MAX);
		}
	}

	void EntityInspectorPanel::ProcessFloatingPointData(entt::meta_data data, entt::meta_any instance)
	{
		if (IsTypeEqual<float>(data.type()))
		{
			ProcessScalarNData<float>(data, instance, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		}
		else if (IsTypeEqual<double>(data.type()))
		{
			ProcessScalarNData<double>(data, instance, ImGuiDataType_Double, -DBL_MAX, DBL_MAX, "%.3f");
		}
	}

	void EntityInspectorPanel::ProcessEnumData(entt::meta_data data, entt::meta_any instance)
	{
		// Get current enum value name by iterating all enum values and comparing
		const char* currentValueName = nullptr;
		auto currentValue = data.get(instance);
		data.type().data([currentValue, &currentValueName](entt::meta_data enumData)
		{
			if (currentValue == enumData.get({}))
			{
				auto valueName = GetPropValue<const char*>(PropertyType::Name, enumData);
				currentValueName = *valueName;
			}
		});

		auto dataName = GetPropValue<const char*>(PropertyType::Name, data);
		if (ImGui::BeginCombo(*dataName, currentValueName))
		{
			// Iterate to display all enum values
			data.type().data([this, data, instance](entt::meta_data enumData)
			{
				auto valueName = GetPropValue<const char*>(PropertyType::Name, enumData);
				bool bIsSelected = ImGui::Selectable(*valueName);
				ShowPropertyTooltip(enumData);
				if (bIsSelected)
				{
					ImGui::SetItemDefaultFocus();
					auto currentValue = data.get(instance);
					auto newValue = enumData.get({});
					if (newValue != currentValue)
					{
						ZE_TRACE("Value changed after edit!");
					}
					SetDataValue(data, instance, newValue);
				}
			});
			ImGui::EndCombo();
		}
	}

	void EntityInspectorPanel::ProcessOtherData(entt::meta_data data, entt::meta_any instance)
	{
		if (IsTypeEqual<std::string>(data.type()))
		{
			ProcessStringData(data, instance);
		}
		else if (IsTypeEqual<glm::vec2>(data.type()))
		{
			ProcessScalarNData<glm::vec2, 2, float>(data, instance, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		}
		else if (IsTypeEqual<glm::vec3>(data.type()))
		{
			ProcessScalarNData<glm::vec3, 3, float>(data, instance, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f");
		}
		else if (IsTypeEqual<glm::vec4>(data.type()))
		{
			ProcessColorData(data, instance);
		}
	}

	void EntityInspectorPanel::ProcessBoolData(entt::meta_data data, entt::meta_any instance)
	{
		auto& boolRef = GetDataValueByRef<bool>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		bool boolCopy;
		bool bUseCopy = DoesPropExist(PropertyType::SetterAndGetter, data);
		if (bUseCopy)
		{
			boolCopy = GetDataValueByRef<bool>(data, instance);
		}

		if (ImGui::Checkbox(*name, bUseCopy ? &boolCopy : &boolRef))
		{
			if (bUseCopy)
			{
				SetDataValue(data, instance, boolCopy);
			}

			ZE_TRACE("Value changed!");
		}
	}

	void EntityInspectorPanel::ProcessStringData(entt::meta_data data, entt::meta_any instance)
	{
		// Map from id to string cache plus a bool flag indicating if we are editing the text
		static std::unordered_map<uint32_t, std::pair<bool, std::string>> stringBuffers;
		auto& stringRef = GetDataValueByRef<std::string>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		auto id = GetUniqueDataID(data);
		std::string stringCopy;
		bool bUseCopy = DoesPropExist(PropertyType::SetterAndGetter, data);
		if (bUseCopy)
		{
			stringCopy = GetDataValueByRef<std::string>(data, instance);
		}

		std::string* stringPtr = bUseCopy ? &stringCopy : &stringRef;
		ImGui::InputText(*name, stringBuffers[id].first ? &stringBuffers[id].second : stringPtr, ImGuiInputTextFlags_AutoSelectAll);
		if (bUseCopy && !stringBuffers[id].first)
		{
			SetDataValue(data, instance, std::move(stringCopy));
		}

		// Write changes to cache first
		if (ImGui::IsItemActivated())
		{
			stringBuffers[id].first = true;
			stringBuffers[id].second = bUseCopy ? stringCopy : stringRef;
		}

		bool bIsValueChanged = false;
		// Apply cache when user finishes editing
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			bIsValueChanged = stringBuffers[id].second != (bUseCopy ? stringCopy : stringRef);
			stringBuffers[id].first = false;
			SetDataValue(data, instance, std::move(stringBuffers[id].second));
		}

		if (bIsValueChanged)
		{
			ZE_TRACE("Value changed after edit!");
		}
	}

	void EntityInspectorPanel::ProcessColorData(entt::meta_data data, entt::meta_any instance)
	{
		// Map from id to value cache plus a bool flag indicating if displayed value is retrieved from cache
		static std::unordered_map<uint32_t, std::pair<bool, glm::vec4>> vec4Buffers;
		auto& vec4Ref = GetDataValueByRef<glm::vec4>(data, instance);
		auto name = GetPropValue<const char*>(PropertyType::Name, data);
		auto id = GetUniqueDataID(data);
		glm::vec4 vec4Copy;
		bool bUseCopy = DoesPropExist(PropertyType::SetterAndGetter, data);
		if (bUseCopy)
		{
			vec4Copy = GetDataValueByRef<glm::vec4>(data, instance);
		}

		float* vec4Ptr = bUseCopy ? glm::value_ptr(vec4Copy) : glm::value_ptr(vec4Ref);
		bool bResult = ImGui::ColorEdit4(*name, vec4Buffers[id].first ? glm::value_ptr(vec4Buffers[id].second) : vec4Ptr);
		if (bUseCopy && !vec4Buffers[id].first)
		{
			SetDataValue(data, instance, std::move(vec4Copy));
		}

		bool bIsValueChangedAfterEdit = false;
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			bIsValueChangedAfterEdit = vec4Buffers[id].second != (bUseCopy ? vec4Copy : vec4Ref);
			if (vec4Buffers[id].first)
			{
				// Apply cache when input box is inactive
				// Dragging will not go here
				SetDataValue(data, instance, std::move(vec4Buffers[id].second));
			}
			vec4Buffers[id].first = false;
		}

		if (ImGui::IsItemActivated())
		{
			//  Update cache when this item is activated
			vec4Buffers[id].second = bUseCopy ? vec4Copy : vec4Ref;

			ImGuiContext* context = ImGui::GetCurrentContext();
			// Input box is activated by double clicking, CTRL-clicking or being tabbed in
			if (ImGui::IsMouseDoubleClicked(0) ||
				(context->IO.KeyCtrl && ImGui::IsMouseClicked(0)) ||
				context->NavJustTabbedId == context->ActiveId)
			{
				// Keep writing to cache as long as input box is active
				vec4Buffers[id].first = true;
			}
		}

		// Value changed during dragging
		if (bResult && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			ZE_TRACE("Value changed!");
		}

		// Value changed after dragging or inputting
		if (bIsValueChangedAfterEdit)
		{
			ZE_TRACE("Value changed after edit!");
		}
	}

}
