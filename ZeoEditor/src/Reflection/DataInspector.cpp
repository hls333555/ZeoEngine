#include "Reflection/DataInspector.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Core/KeyCodes.h"
#include "Panels/DataInspectorPanel.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/GameFramework/Components.h"

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
						bool bIsSeqContainer = data.type().is_sequence_container();
						bool bIsAssContainer = data.type().is_associative_container();
						ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
						if (bIsSeqContainer)
						{
							auto size = data.get(instance).as_sequence_container().size();
							flags = size > 0 ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
						}
						if (bIsAssContainer)
						{
							auto size = data.get(instance).as_associative_container().size();
							flags = size > 0 ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
						}
						auto dataName = GetMetaObjectDisplayName(data);
						// Data name
						bool bIsContainerExpanded = ImGui::TreeNodeEx(*dataName, flags);
						// Data tooltip
						ShowPropertyTooltip(data);
						// Switch to the right column
						ImGui::NextColumn();
						if (bIsSeqContainer || bIsAssContainer)
						{
							// Insert and erase all buttons
							DrawSeqButtons(data, instance);
						}
						// Push data name as id
						ImGui::PushID(*dataName);
						{
							if (bIsSeqContainer)
							{
								if (bIsContainerExpanded)
								{
									EvaluateSequenceContainerData(data, instance);

									ImGui::TreePop();
								}
							}
							else if (bIsAssContainer)
							{
								if (bIsContainerExpanded)
								{
									EvaluateAssociativeContainerData(data, instance);

									ImGui::TreePop();
								}
							}
							else
							{
								// Align width to the right side
								ImGui::SetNextItemWidth(-1.0f);
								EvaluateData(data, instance);
							}
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

	static entt::meta_sequence_container::iterator InsertDefaultValueForSeq(entt::meta_data data, entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it)
	{
		//TODO: FIXME
		// "0" value works for "all" types because we have registered their conversion functions
		auto& [retIt, res] = seqView.insert(it, 0);
		if (res)
		{
			ZE_TRACE("Value changed after edit!");
		}
		else
		{
			// For special types like user-defined enums, we have to invoke a function instead
			auto defaultValue = CreateTypeDefaultValue(seqView.value_type());
			auto& [retIt, res] = seqView.insert(it, defaultValue);
			if (res)
			{
				ZE_TRACE("Value changed after edit!");
			}
			else
			{
				auto dataName = GetMetaObjectDisplayName(data);
				ZE_CORE_ERROR("Failed to insert with data: '{0}'!", *dataName);
			}
		}
		return retIt;
	}

	static entt::meta_sequence_container::iterator EraseValueForSeq(entt::meta_data data, entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it)
	{
		auto& [retIt, res] = seqView.erase(it);
		if (res)
		{
			ZE_TRACE("Value changed after edit!");
		}
		else
		{
			auto dataName = GetMetaObjectDisplayName(data);
			ZE_CORE_ERROR("Failed to erase with data: {0}!", *dataName);
		}
		return retIt;
	}

	void DataInspector::DrawSeqButtons(entt::meta_data data, entt::meta_any instance)
	{
		auto& seqView = data.get(instance).as_sequence_container();

		ImGui::Text("%d elements", seqView.size());
		ImGui::SameLine();
		if (ImGui::BeginCombo("##SequenceContainerOperation", nullptr, ImGuiComboFlags_NoPreview))
		{
			if (ImGui::Selectable("Add"))
			{
				InsertDefaultValueForSeq(data, seqView, seqView.end());
			}
			if (ImGui::Selectable("Clear"))
			{
				if (seqView.size() > 0 && seqView.clear())
				{
					ZE_TRACE("Value changed after edit!");
				}
			}

			ImGui::EndCombo();
		}
	}

	void DataInspector::EvaluateSequenceContainerData(entt::meta_data data, entt::meta_any instance)
	{
		auto& seqView = data.get(instance).as_sequence_container();
		const auto type = seqView.value_type();
		uint32_t i = 0;
		for (auto it = seqView.begin(); it != seqView.end();)
		{
			auto element = *it;
			// Switch to the next row
			ImGui::NextColumn();
			// Data index
			ImGui::TreeNodeEx(std::to_string(i).c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
			// Switch to the right column
			ImGui::NextColumn();
			// Push container index
			ImGui::PushID(i);
			{
				// Make sure element widget + dropdown button can reach desired size
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight);
				if (type.is_integral())
				{
					EvaluateIntegralData(data, element, true);
				}
				else if (type.is_floating_point())
				{
					EvaluateFloatingPointData(data, element, true);
				}
				else if (type.is_enum())
				{
					ProcessEnumData(data, element, true);
				}
				else
				{
					EvaluateOtherData(data, element, true);
				}
				ImGui::SameLine();
				if (ImGui::BeginCombo("##SequenceElementOperation", nullptr, ImGuiComboFlags_NoPreview))
				{
					if (ImGui::Selectable("Insert"))
					{
						it = InsertDefaultValueForSeq(data, seqView, it);
					}
					if (ImGui::Selectable("Erase"))
					{
						it = EraseValueForSeq(data, seqView, it);
					}

					ImGui::EndCombo();
				}
			}
			ImGui::PopID();

			if (it != seqView.end())
			{
				++it, ++i;
			}
		}
	}

	void DataInspector::EvaluateAssociativeContainerData(entt::meta_data data, entt::meta_any instance)
	{
		
	}

	void DataInspector::EvaluateData(entt::meta_data data, entt::meta_any instance)
	{
		if (data.type().is_integral())
		{
			EvaluateIntegralData(data, instance, false);
		}
		else if (data.type().is_floating_point())
		{
			EvaluateFloatingPointData(data, instance, false);
		}
		else if (data.type().is_enum())
		{
			ProcessEnumData(data, instance, false);
		}
		else
		{
			EvaluateOtherData(data, instance, false);
		}
	}

	void DataInspector::EvaluateIntegralData(entt::meta_data data, entt::meta_any instance, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<bool>(type))
		{
			ProcessBoolData(data, instance, bIsSeqContainer);
		}
		else if (IsTypeEqual<int8_t>(type))
		{
			ProcessScalarNData<int8_t>(data, instance, bIsSeqContainer, ImGuiDataType_S8, static_cast<int8_t>(INT8_MIN), static_cast<int8_t>(INT8_MAX), "%hhd");
		}
		else if (IsTypeEqual<int32_t>(type))
		{
			ProcessScalarNData<int32_t>(data, instance, bIsSeqContainer, ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d");
		}
		else if (IsTypeEqual<int64_t>(type))
		{
			ProcessScalarNData<int64_t>(data, instance, bIsSeqContainer, ImGuiDataType_S64, INT64_MIN, INT64_MAX, "%lld");
		}
		else if (IsTypeEqual<uint8_t>(type))
		{
			ProcessScalarNData<uint8_t>(data, instance, bIsSeqContainer, ImGuiDataType_U8, 0ui8, UINT8_MAX, "%hhu");
		}
		else if (IsTypeEqual<uint32_t>(type))
		{
			ProcessScalarNData<uint32_t>(data, instance, bIsSeqContainer, ImGuiDataType_U32, 0ui32, UINT32_MAX, "%u");
		}
		else if (IsTypeEqual<uint64_t>(type))
		{
			ProcessScalarNData<uint64_t>(data, instance, bIsSeqContainer, ImGuiDataType_U64, 0ui64, UINT64_MAX, "%llu");
		}
	}

	void DataInspector::EvaluateFloatingPointData(entt::meta_data data, entt::meta_any instance, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<float>(type))
		{
			ProcessScalarNData<float>(data, instance, bIsSeqContainer, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		}
		else if (IsTypeEqual<double>(type))
		{
			ProcessScalarNData<double>(data, instance, bIsSeqContainer, ImGuiDataType_Double, -DBL_MAX, DBL_MAX, "%.3lf");
		}
	}

	void DataInspector::EvaluateOtherData(entt::meta_data data, entt::meta_any instance, bool bIsSeqContainer)
	{
		const auto type = bIsSeqContainer ? instance.type() : data.type();
		if (IsTypeEqual<std::string>(type))
		{
			ProcessStringData(data, instance, bIsSeqContainer);
		}
		else if (IsTypeEqual<glm::vec2>(type))
		{
			ProcessScalarNData<glm::vec2, 2, float>(data, instance, bIsSeqContainer, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		}
		else if (IsTypeEqual<glm::vec3>(type))
		{
			ProcessScalarNData<glm::vec3, 3, float>(data, instance, bIsSeqContainer, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		}
		else if (IsTypeEqual<glm::vec4>(type))
		{
			ProcessColorData(data, instance, bIsSeqContainer);
		}
		else if (IsTypeEqual<Ref<Texture2D>>(type))
		{
			ProcessTexture2DData(data, instance, bIsSeqContainer);
		}
	}

	void DataInspector::ProcessBoolData(entt::meta_data data, entt::meta_any instance, bool bIsSeqContainer)
	{
		auto& boolRef = bIsSeqContainer ? instance.cast<bool>() : GetDataValueByRef<bool>(data, instance);
		bool bUseCopy = DoesPropExist(PropertyType::SetterAndGetter, data);
		bool boolCopy = bUseCopy ? GetDataValue<bool>(data, instance) : false;

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

	void DataInspector::ProcessEnumData(entt::meta_data data, entt::meta_any instance, bool bIsSeqContainer)
	{
		auto enumValue = bIsSeqContainer ? instance : data.get(instance);
		const char* currentValueName = GetEnumDisplayName(enumValue);

		// NOTE: We cannot leave ComboBox's label empty
		if (ImGui::BeginCombo("##Enum", currentValueName))
		{
			// TODO: Reverse enum display order
			// Iterate to display all enum values
			const auto& datas = bIsSeqContainer ? instance.type().data() : data.type().data();
			for (auto enumData : datas)
			{
				auto valueName = GetMetaObjectDisplayName(enumData);
				bool bIsSelected = ImGui::Selectable(*valueName);
				ShowPropertyTooltip(enumData);
				if (bIsSelected)
				{
					ImGui::SetItemDefaultFocus();
					auto newValue = enumData.get({});
					if (newValue != enumValue)
					{
						if (bIsSeqContainer)
						{
							SetEnumValueForSeq(instance, newValue);
						}
						else
						{
							SetDataValue(data, instance, newValue);
						}
						ZE_TRACE("Value changed after edit!");
					}
				}
			}

			ImGui::EndCombo();
		}
	}

	void DataInspector::ProcessStringData(entt::meta_data data, entt::meta_any instance, bool bIsSeqContainer)
	{
		// Map from id to string cache plus a bool flag indicating if we are editing the text
		static std::unordered_map<uint32_t, std::pair<bool, std::string>> stringBuffers;
		auto& stringRef = bIsSeqContainer ? instance.cast<std::string>() : GetDataValueByRef<std::string>(data, instance);
		auto id = GetUniqueDataID(data);
		bool bUseCopy = DoesPropExist(PropertyType::SetterAndGetter, data);
		std::string stringCopy = bUseCopy ? GetDataValue<std::string>(data, instance) : "";

		std::string* stringPtr = bUseCopy ? &stringCopy : &stringRef;
		// NOTE: We cannot leave InputBox's label empty
		ImGui::InputText("##String", stringBuffers[id].first ? &stringBuffers[id].second : stringPtr, ImGuiInputTextFlags_AutoSelectAll);
		if (bUseCopy && !stringBuffers[id].first)
		{
			SetDataValue(data, instance, stringCopy);
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
			if (bIsSeqContainer)
			{
				stringRef = std::move(stringBuffers[id].second);
			}
			else
			{
				SetDataValue(data, instance, std::move(stringBuffers[id].second));
			}
		}

		if (bIsValueChanged)
		{
			ZE_TRACE("Value changed after edit!");
		}
	}

	void DataInspector::ProcessColorData(entt::meta_data data, entt::meta_any instance, bool bIsSeqContainer)
	{
		// Map from id to value cache plus a bool flag indicating if displayed value is retrieved from cache
		static std::unordered_map<uint32_t, std::pair<bool, glm::vec4>> vec4Buffers;
		auto& vec4Ref = bIsSeqContainer ? instance.cast<glm::vec4>() : GetDataValueByRef<glm::vec4>(data, instance);
		auto id = GetUniqueDataID(data);
		bool bUseCopy = DoesPropExist(PropertyType::SetterAndGetter, data);
		glm::vec4 vec4Copy = bUseCopy ? GetDataValue<glm::vec4>(data, instance) : glm::vec4();

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
				if (bIsSeqContainer)
				{
					vec4Ref = std::move(vec4Buffers[id].second);
				}
				else
				{
					SetDataValue(data, instance, std::move(vec4Buffers[id].second));
				}
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

	void DataInspector::ProcessTexture2DData(entt::meta_data data, entt::meta_any instance, bool bIsSeqContainer)
	{
		auto& texture2DRef = bIsSeqContainer ? instance.cast<Ref<Texture2D>>() : GetDataValueByRef<Ref<Texture2D>>(data, instance);

		Texture2DLibrary& library = Texture2DLibrary::Get();
		// Texture preview
		{
			auto backgroundTexture = library.Get("assets/textures/Checkerboard_Alpha.png");
			const float texturePreviewWidth = 75.0f;
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
		if (bIsSeqContainer)
		{
			// Make sure browser widget + dropdown button can reach desired size
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight);
		}
		else
		{
			// Align width to the right side
			ImGui::SetNextItemWidth(-1.0f);
		}

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
					texture2DRef = loadedTexture;
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
					texture2DRef = texture;
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
