#include "Reflection/DataInspector.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Core/KeyCodes.h"
#include "Panels/DataInspectorPanel.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Utils/PlatformUtils.h"

namespace ZeoEngine {

	DataInspector::DataInspector(DataInspectorPanel* context)
		:m_Context(context)
	{
	}

	bool DataInspector::ProcessType(entt::meta_type type, Entity entity)
	{
		const auto instance = GetTypeInstance(type, m_Context->GetScene()->m_Registry, entity);
		auto typeName = GetMetaObjectDisplayName(type);

		// Do not show CollapsingHeader if PropertyType::HideTypeHeader is set
		bool bShouldDisplayHeader = !DoesPropExist(PropertyType::HideTypeHeader, type);
		bool bIsHeaderExpanded = true;
		bool bWillRemoveType = false;
		if (bShouldDisplayHeader)
		{
			ImGui::Columns(1);
			// Get available content region before adding CollapsingHeader as it will occupy space
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			// Type collapsing header
			bIsHeaderExpanded = ImGui::CollapsingHeader(*typeName, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
			// Type tooltip
			ShowPropertyTooltip(type);

			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);

			// Component settings button
			{
				if (ImGui::Button("...", { lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}

				if (ImGui::BeginPopup("ComponentSettings"))
				{
					// Inherent types can never be removed
					auto bIsInherentType = DoesPropExist(PropertyType::InherentType, type);
					if (ImGui::MenuItem("Remove Component", nullptr, false, !bIsInherentType))
					{
						bWillRemoveType = true;
					}

					ImGui::EndPopup();
				}
			}
		}
		if (bIsHeaderExpanded)
		{
			if (m_bIsPreprocessedDatasDirty)
			{
				ZE_CORE_TRACE("Sorting datas on '{0}'", *typeName);

				// Iterate all datas on this entity, reverse their order and categorize them
				for (auto data : type.data())
				{
					PreprocessData(type, data);
				}
			}

			for (const auto& [category, datas] : m_PreprocessedDatas[type.type_id()])
			{
				bool bShouldDisplayTree = false;
				std::list<entt::meta_data> visibleDatas;
				// Do not show TreeNode if none of these datas will show or category is not set
				for (const auto data : datas)
				{
					if (!ShouldHideData(data, instance))
					{
						bShouldDisplayTree = category != "";
						visibleDatas.push_back(data);
					}
				}
				bool bIsTreeExpanded = true;
				if (bShouldDisplayTree)
				{
					ImGui::Columns(1);
					// Data category tree
					bIsTreeExpanded = ImGui::TreeNodeEx(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen/* Prevent indent of next row, which will affect column. */);
				}
				if (bIsTreeExpanded)
				{
					// We want column seperator to keep synced across different entities
					ImGui::PopID();
					ImGui::PopID();
					ImGui::Columns(2);
					// Re-push entity id
					ImGui::PushID(static_cast<uint32_t>(entity));
					// Re-push type id
					ImGui::PushID(type.type_id());

					ImGui::AlignTextToFramePadding();
					for (const auto data : visibleDatas)
					{
						auto dataName = GetMetaObjectDisplayName(data);
						// Data name
						ImGui::TreeNodeEx(*dataName, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
						// Data tooltip
						ShowPropertyTooltip(data);
						// Switch to the right column
						ImGui::NextColumn();
						// Align width to the right side
						ImGui::SetNextItemWidth(-1.0f);
						// Push data name as id
						ImGui::PushID(*dataName);
						{
							EvaluateData(data, instance);
						}
						ImGui::PopID();
						// Switch to the next row
						ImGui::NextColumn();
					}
				}
			}
		}

		if (bWillRemoveType)
		{
			entity.RemoveType(type, m_Context->GetScene()->m_Registry);
		}

		return bWillRemoveType;
	}

	void DataInspector::MarkPreprocessedDatasClean()
	{
		m_bIsPreprocessedDatasDirty = false;
	}

	void DataInspector::MarkPreprocessedDatasDirty()
	{
		m_PreprocessedDatas.clear();
		m_bIsPreprocessedDatasDirty = true;
	}

	uint32_t DataInspector::GetUniqueDataID(entt::meta_data data)
	{
		return ImGui::GetCurrentWindow()->GetID(data.id());
	}

	void DataInspector::PreprocessData(entt::meta_type type, entt::meta_data data)
	{
		auto categoryName = GetPropValue<const char*>(PropertyType::Category, data);
		const char* category = categoryName ? *categoryName : "";
		// Reverse data display order and categorize them
		m_PreprocessedDatas[type.type_id()][category].push_front(data);
	}

	bool DataInspector::ShouldHideData(entt::meta_data data, entt::meta_any instance)
	{
		auto bIsHiddenInEditor = DoesPropExist(PropertyType::HiddenInEditor, data);
		if (bIsHiddenInEditor) return true;

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

			auto keyData = entt::resolve_type(instance.type().type_id()).data(entt::hashed_string::value(keyStr.c_str()));
			auto keyDataValue = keyData.get(instance);
			auto keyDataType = keyData.type();
			auto valueToCompare = keyDataType.data(entt::hashed_string::value(valueStr.c_str())).get({});
			return keyDataValue != valueToCompare;
		}

		return false;
	}

	void DataInspector::EvaluateData(entt::meta_data data, entt::meta_any instance)
	{
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
	}

	void DataInspector::ProcessIntegralData(entt::meta_data data, entt::meta_any instance)
	{
		if (IsTypeEqual<bool>(data.type()))
		{
			ProcessBoolData(data, instance);
		}
		else if (IsTypeEqual<int8_t>(data.type()))
		{
			ProcessScalarNData<int8_t>(data, instance, ImGuiDataType_S8, static_cast<int8_t>(INT8_MIN), static_cast<int8_t>(INT8_MAX), "%hhd");
		}
		else if (IsTypeEqual<int32_t>(data.type()))
		{
			ProcessScalarNData<int32_t>(data, instance, ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d");
		}
		else if (IsTypeEqual<int64_t>(data.type()))
		{
			ProcessScalarNData<int64_t>(data, instance, ImGuiDataType_S64, INT64_MIN, INT64_MAX, "%lld");
		}
		else if (IsTypeEqual<uint8_t>(data.type()))
		{
			ProcessScalarNData<uint8_t>(data, instance, ImGuiDataType_U8, 0ui8, UINT8_MAX, "%hhu");
		}
		else if (IsTypeEqual<uint32_t>(data.type()))
		{
			ProcessScalarNData<uint32_t>(data, instance, ImGuiDataType_U32, 0ui32, UINT32_MAX, "%u");
		}
		else if (IsTypeEqual<uint64_t>(data.type()))
		{
			ProcessScalarNData<uint64_t>(data, instance, ImGuiDataType_U64, 0ui64, UINT64_MAX, "%llu");
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
			ProcessScalarNData<double>(data, instance, ImGuiDataType_Double, -DBL_MAX, DBL_MAX, "%.3lf");
		}
	}

	void DataInspector::ProcessEnumData(entt::meta_data data, entt::meta_any instance)
	{
		// Get current enum value name by iterating all enum values and comparing
		const char* currentValueName = nullptr;
		auto currentValue = data.get(instance);
		for (auto enumData : data.type().data())
		{
			if (currentValue == enumData.get({}))
			{
				auto valueName = GetMetaObjectDisplayName(enumData);
				currentValueName = *valueName;
			}
		}

		// NOTE: We cannot leave ComboBox's label empty
		if (ImGui::BeginCombo("##Enum", currentValueName))
		{
			// TODO: Reverse enum display order
			// Iterate to display all enum values
			for (auto enumData : data.type().data())
			{
				auto valueName = GetMetaObjectDisplayName(enumData);
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
			}
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
			ProcessScalarNData<glm::vec3, 3, float>(data, instance, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
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
		bool boolCopy;
		bool bUseCopy = DoesPropExist(PropertyType::SetterAndGetter, data);
		if (bUseCopy)
		{
			boolCopy = GetDataValue<bool>(data, instance);
		}

		// NOTE: We cannot leave Checkbox's label empty
		if (ImGui::Checkbox("##Bool", bUseCopy ? &boolCopy : &boolRef))
		{
			if (bUseCopy)
			{
				SetDataValue(data, instance, boolCopy);
			}

			ZE_TRACE("Value changed after edit!");
		}
	}

	void DataInspector::ProcessStringData(entt::meta_data data, entt::meta_any instance)
	{
		// Map from id to string cache plus a bool flag indicating if we are editing the text
		static std::unordered_map<uint32_t, std::pair<bool, std::string>> stringBuffers;
		auto& stringRef = GetDataValueByRef<std::string>(data, instance);
		auto id = GetUniqueDataID(data);
		std::string stringCopy;
		bool bUseCopy = DoesPropExist(PropertyType::SetterAndGetter, data);
		if (bUseCopy)
		{
			stringCopy = GetDataValue<std::string>(data, instance);
		}

		std::string* stringPtr = bUseCopy ? &stringCopy : &stringRef;
		// NOTE: We cannot leave InputBox's label empty
		ImGui::InputText("##String", stringBuffers[id].first ? &stringBuffers[id].second : stringPtr, ImGuiInputTextFlags_AutoSelectAll);
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
		auto id = GetUniqueDataID(data);
		glm::vec4 vec4Copy;
		bool bUseCopy = DoesPropExist(PropertyType::SetterAndGetter, data);
		if (bUseCopy)
		{
			vec4Copy = GetDataValue<glm::vec4>(data, instance);
		}

		float* vec4Ptr = bUseCopy ? glm::value_ptr(vec4Copy) : glm::value_ptr(vec4Ref);
		bool bResult = ImGui::ColorEdit4("", vec4Buffers[id].first ? glm::value_ptr(vec4Buffers[id].second) : vec4Ptr);
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

		Texture2DLibrary& library = Texture2DLibrary::Get();
		// Texture preview
		{
			auto backgroundTexture = library.Get("assets/textures/Checkerboard_Alpha.png");
			const float texturePreviewWidth = 100.0f;
			// Draw checkerboard texture as background first
			ImGui::GetWindowDrawList()->AddImage(backgroundTexture->GetTexture(),
				ImGui::GetCursorScreenPos(),
				{ ImGui::GetCursorScreenPos().x + texturePreviewWidth, ImGui::GetCursorScreenPos().y + texturePreviewWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f });
			// Draw our texture on top of that
			ImGui::Image(texture2DRef ? texture2DRef->GetTexture() : nullptr,
				{ texturePreviewWidth, texturePreviewWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f },
				texture2DRef ? ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f } : ImVec4{ 1.0f, 1.0f, 1.0f, 0.0f });
			// Display texture info tooltip
			if (texture2DRef && ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Resolution: %dx%d\nHas alpha: %s", texture2DRef->GetWidth(), texture2DRef->GetHeight(), texture2DRef->HasAlpha() ? "true" : "false");
			}
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(-1.0f);

		// Texture browser
		if (ImGui::BeginCombo("##Texture2D", texture2DRef ? texture2DRef->GetFileName().c_str() : nullptr))
		{
			bool bIsValueChangedAfterEdit = false;
			// Pop up file browser to select a texture from disk
			if (ImGui::Selectable("Browse texture..."))
			{
				// TODO: Support more texture format
				auto filePath = FileDialogs::OpenFile("PNG (*.png)\0*.png");
				if (filePath)
				{
					// Add selected texture to the library
					Ref<Texture2D> loadedTexture = library.GetOrLoad(*filePath);
					bIsValueChangedAfterEdit = loadedTexture != texture2DRef;
					SetDataValue(data, instance, loadedTexture);
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
