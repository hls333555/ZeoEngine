#include "Reflection/DataInspector.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include <nfd.h>

#include "Engine/Core/KeyCodes.h"
#include "Panels/DataInspectorPanel.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	DataInspector::DataInspector(DataInspectorPanel* context)
		:m_Context(context)
	{
	}

	uint32_t DataInspector::GetUniqueDataID(entt::meta_data data)
	{
		return ImGui::GetCurrentWindow()->GetID(data.id());
	}

	bool DataInspector::ShouldHideData(entt::meta_data data, entt::meta_any instance)
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

	void DataInspector::ProcessType(entt::meta_type type, Entity entity)
	{
		const auto instance = GetTypeInstance(type, m_Context->GetScene()->m_Registry, entity);
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

	void DataInspector::ProcessIntegralData(entt::meta_data data, entt::meta_any instance)
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

	void DataInspector::ProcessFloatingPointData(entt::meta_data data, entt::meta_any instance)
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

	void DataInspector::ProcessEnumData(entt::meta_data data, entt::meta_any instance)
	{
		auto dataName = GetPropValue<const char*>(PropertyType::Name, data);

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

	void DataInspector::ProcessOtherData(entt::meta_data data, entt::meta_any instance)
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
		else if (IsTypeEqual<Ref<Texture2D>>(data.type()))
		{
			ProcessTexture2DData(data, instance);
		}
	}

	void DataInspector::ProcessBoolData(entt::meta_data data, entt::meta_any instance)
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

	void DataInspector::ProcessStringData(entt::meta_data data, entt::meta_any instance)
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

	void DataInspector::ProcessColorData(entt::meta_data data, entt::meta_any instance)
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

	void DataInspector::ProcessTexture2DData(entt::meta_data data, entt::meta_any instance)
	{
		auto& texture2DRef = GetDataValueByRef<Ref<Texture2D>>(data, instance);
		auto dataName = GetPropValue<const char*>(PropertyType::Name, data);

		Texture2DLibrary& library = Texture2DLibrary::Get();
		// Texture preview
		{
			auto backgroundTexture = library.Get("../ZeoEditor/assets/textures/Checkerboard_Alpha.png");
			const float texturePreviewWidth = 100.0f;
			// Draw checkerboard texture as background first
			ImGui::GetWindowDrawList()->AddImage(backgroundTexture->GetTexture(),
				ImGui::GetCursorScreenPos(),
				ImVec2(ImGui::GetCursorScreenPos().x + texturePreviewWidth, ImGui::GetCursorScreenPos().y + texturePreviewWidth),
				ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			// Draw our texture on top of that
			ImGui::Image(texture2DRef ? texture2DRef->GetTexture() : nullptr,
				ImVec2(texturePreviewWidth, texturePreviewWidth),
				ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f),
				texture2DRef ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
			// Display texture info tooltip
			if (texture2DRef && ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Resolution: %dx%d\nHas alpha: %s", texture2DRef->GetWidth(), texture2DRef->GetHeight(), texture2DRef->HasAlpha() ? "true" : "false");
			}
		}

		ImGui::SameLine();

		// Texture browser
		if (ImGui::BeginCombo(*dataName, texture2DRef ? texture2DRef->GetFileName().c_str() : nullptr))
		{
			bool bIsValueChangedAfterEdit = false;
			// Pop up file browser to select a texture from disk
			if (ImGui::Selectable("Browse texture..."))
			{
				nfdchar_t* outPath = nullptr;
				// TODO: Support more texture format
				nfdresult_t result = NFD_OpenDialog("png", nullptr, &outPath);
				if (result == NFD_OKAY)
				{
					// Add selected texture to the library
					Ref<Texture2D> loadedTexture = library.GetOrLoad(outPath);
					bIsValueChangedAfterEdit = loadedTexture != texture2DRef;
					SetDataValue(data, instance, loadedTexture);
					free(outPath);
				}
				else if (result == NFD_ERROR)
				{
					ZE_CORE_ERROR("ProcessTexture2DData: {0}", NFD_GetError());
				}
			}
			ImGui::Separator();
			// List all loaded textures from Texture2DLibrary
			for (const auto& [path, texture] : library.GetTexturesMap())
			{
				ImGui::PushID(texture->GetPath().c_str());

				const float textureThumbnailWidth = 30.0f;
				bool bIsSelected = ImGui::Selectable("##TextureDropdownThumbnail", false, 0, ImVec2(0.0f, textureThumbnailWidth));
				// Display texture path tooltip for drop-down item
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", texture->GetPath().c_str());
				}
				ImGui::SameLine();
				// Draw texture thumbnail
				ImGui::Image(texture->GetTexture(),
					ImVec2(textureThumbnailWidth, textureThumbnailWidth),
					ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				// Display texture name
				ImGui::Text(texture->GetFileName().c_str());
				if (bIsSelected)
				{
					bIsValueChangedAfterEdit = texture != texture2DRef;
					SetDataValue(data, instance, texture);
				}

				ImGui::PopID();
			}
			ImGui::EndCombo();

			if (bIsValueChangedAfterEdit)
			{
				ZE_TRACE("Value changed after edit!");
			}
		}

		// Display texture path tooltip for current selection
		if (texture2DRef && ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", texture2DRef->GetPath().c_str());
		}
	}

}
