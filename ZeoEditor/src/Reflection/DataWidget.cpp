#include "Reflection/DataWidget.h"

#include <misc/cpp/imgui_stdlib.h>

#include "Engine/Utils/PlatformUtils.h"

namespace ZeoEngine {

	Ref<DataWidget> ConstructBasicDataWidget(const DataSpec& dataSpec)
	{
		const bool bIsSeqElement = dataSpec.bIsSeqElement;
		const auto type = bIsSeqElement ? dataSpec.SeqView.value_type() : dataSpec.Data.type();
		switch (EvaluateMetaType(type))
		{
		case BasicMetaType::SEQCON:
			if (bIsSeqElement)
			{
				ZE_CORE_ERROR("Container nesting is not supported!");
				return {};
			}
			return CreateRef<SequenceContainerWidget>(dataSpec);
		case BasicMetaType::ASSCON:
			if (bIsSeqElement)
			{
				ZE_CORE_ERROR("Container nesting is not supported!");
				return {};
			}
			return CreateRef<AssociativeContainerWidget>(dataSpec);
		case BasicMetaType::BOOL:
			return CreateRef<BoolDataWidget>(dataSpec);
		case BasicMetaType::I8:
			return CreateRef<ScalarNDataWidget<int8_t>>(dataSpec, ImGuiDataType_S8, static_cast<int8_t>(INT8_MIN), static_cast<int8_t>(INT8_MAX), "%hhd");
		case BasicMetaType::I32:
			return CreateRef<ScalarNDataWidget<int32_t>>(dataSpec, ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d");
		case BasicMetaType::I64:
			return CreateRef<ScalarNDataWidget<int64_t>>(dataSpec, ImGuiDataType_S64, INT64_MIN, INT64_MAX, "%lld");
		case BasicMetaType::UI8:
			return CreateRef<ScalarNDataWidget<uint8_t>>(dataSpec, ImGuiDataType_U8, 0ui8, UINT8_MAX, "%hhu");
		case BasicMetaType::UI32:
			return CreateRef<ScalarNDataWidget<uint32_t>>(dataSpec, ImGuiDataType_U32, 0ui32, UINT32_MAX, "%u");
		case BasicMetaType::UI64:
			return CreateRef<ScalarNDataWidget<uint64_t>>(dataSpec, ImGuiDataType_U64, 0ui64, UINT64_MAX, "%llu");
		case BasicMetaType::FLOAT:
			return CreateRef<ScalarNDataWidget<float>>(dataSpec, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		case BasicMetaType::DOUBLE:
			return CreateRef<ScalarNDataWidget<double>>(dataSpec, ImGuiDataType_Double, -DBL_MAX, DBL_MAX, "%.3lf");
		case BasicMetaType::ENUM:
			return CreateRef<EnumDataWidget>(dataSpec);
		case BasicMetaType::STRING:
			return CreateRef<StringDataWidget>(dataSpec);
		case BasicMetaType::VEC2:
			return CreateRef<ScalarNDataWidget<glm::vec2, 2, float>>(dataSpec, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		case BasicMetaType::VEC3:
			return CreateRef<ScalarNDataWidget<glm::vec3, 3, float>>(dataSpec, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		case BasicMetaType::VEC4:
			return CreateRef<ColorDataWidget>(dataSpec);
		case BasicMetaType::TEXTURE:
			return CreateRef<Texture2DDataWidget>(dataSpec);
		case BasicMetaType::PARTICLE:
			// TODO:
			return {};
		}

		return {};
	}

	BoolDataWidget::BoolDataWidget(const DataSpec& dataSpec)
	{
		Init(dataSpec);
	}

	void BoolDataWidget::Draw(const entt::meta_any& compInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, elementIndex)) return;

		if (ImGui::Checkbox("##Bool", &m_Buffer))
		{
			SetValueToData();
			//InvokePostDataValueChangeCallback();
		}

		PostDraw();
	}

	EnumDataWidget::EnumDataWidget(const DataSpec& dataSpec)
	{
		Init(dataSpec);
		InitEnumDatas();
	}

	void EnumDataWidget::Draw(const entt::meta_any& compInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, elementIndex)) return;

		if (ImGui::BeginCombo("##Enum", m_CurrentEnumDataName))
		{
			for (const auto enumData : m_EnumDatas)
			{
				auto enumDataName = GetMetaObjectDisplayName(enumData);
				bool bIsSelected = ImGui::Selectable(*enumDataName);
				ShowPropertyTooltip(enumData);
				if (bIsSelected)
				{
					ImGui::SetItemDefaultFocus();
					m_Buffer = enumData.get({});
					if (IsBufferChanged())
					{
						if (m_DataSpec.bIsSeqElement)
						{
							// TODO: Think a better way for enum sequence container
							SetEnumValueForSeq(m_DataSpec.SeqView[m_DataSpec.ElementIndex], m_Buffer);
						}
						else
						{
							SetValueToData();
						}
						//InvokePostDataValueChangeCallback();
					}
				}
			}

			ImGui::EndCombo();
		}

		PostDraw();
	}

	void EnumDataWidget::InitEnumDatas()
	{
		m_EnumDatas.clear();
		auto type = m_DataSpec.bIsSeqElement ? m_DataSpec.SeqView.value_type() : m_Buffer.type();
		for (const auto enumData : type.data())
		{
			// Reverse enum data order
			m_EnumDatas.push_front(enumData);
		}
	}

	void EnumDataWidget::UpdateBuffer()
	{
		m_Buffer = GetValueFromData();
		m_CurrentEnumDataName = GetEnumDisplayName(m_Buffer);
	}

	StringDataWidget::StringDataWidget(const DataSpec& dataSpec)
	{
		Init(dataSpec);
	}

	void StringDataWidget::Draw(const entt::meta_any& compInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, elementIndex)) return;

		ImGui::InputText("##String", &m_Buffer, ImGuiInputTextFlags_AutoSelectAll);
		// For tabbing (we must force set value back in this case or the buffer will be reset on the next draw)
		if (ImGui::IsKeyPressed(Key::Tab) && ImGui::GetFocusID() == ImGui::GetItemID())
		{
			SetValueToData();
			//InvokePostDataValueChangeCallback();
		}
		if (ImGui::IsItemActivated())
		{
			UpdateBuffer();
		}
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (IsBufferChanged())
			{
				SetValueToData();
				//InvokePostDataValueChangeCallback();
			}
		}

		PostDraw();
	}

	ColorDataWidget::ColorDataWidget(const DataSpec& dataSpec)
	{
		Init(dataSpec);
	}

	void ColorDataWidget::Draw(const entt::meta_any& compInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, elementIndex)) return;

		bool bChanged = ImGui::ColorEdit4("", glm::value_ptr(m_Buffer));
		
		if (
			// For dragging
			bChanged && ImGui::IsMouseDragging(ImGuiMouseButton_Left) ||
			// For tabbing (we must force set value back in this case or the buffer will be reset on the next draw)
			ImGui::IsKeyPressed(Key::Tab) && ImGui::GetFocusID() == ImGui::GetItemID())
		{
			SetValueToData();
			//InvokeOnDataValueChangeCallback();
		}
		// For multi-component widget, tabbing will switch to the next component, so we must handle deactivation to apply cache first
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (IsBufferChanged())
			{
				SetValueToData();
				//InvokePostDataValueChangeCallback();
			}
		}
		if (ImGui::IsItemActivated())
		{
			UpdateBuffer();
		}

		PostDraw();
	}

	Texture2DDataWidget::Texture2DDataWidget(const DataSpec& dataSpec)
	{
		Init(dataSpec);
	}

	void Texture2DDataWidget::Draw(const entt::meta_any& compInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, elementIndex)) return;

		Texture2DLibrary& library = Texture2DLibrary::Get();
		// Texture preview
		{
			auto backgroundTexture = library.Get("assets/textures/Checkerboard_Alpha.png");
			constexpr float texturePreviewWidth = 75.0f;
			// Draw checkerboard texture as background first
			ImGui::GetWindowDrawList()->AddImage(backgroundTexture->GetTexture(),
				ImGui::GetCursorScreenPos(),
				{ ImGui::GetCursorScreenPos().x + texturePreviewWidth, ImGui::GetCursorScreenPos().y + texturePreviewWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f });
			// Draw our texture on top of that
			ImGui::Image(m_Buffer ? m_Buffer->GetTexture() : nullptr,
				{ texturePreviewWidth, texturePreviewWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f },
				m_Buffer ? ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f } : ImVec4{ 1.0f, 1.0f, 1.0f, 0.0f });
			// Display texture info tooltip
			if (m_Buffer && ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Resolution: %dx%d\nHas alpha: %s", m_Buffer->GetWidth(), m_Buffer->GetHeight(), m_Buffer->HasAlpha() ? "true" : "false");
			}
		}

		ImGui::SameLine();

		if (m_DataSpec.bIsSeqElement)
		{
			// Make sure browser widget + dropdown button can reach desired size
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 5.0f;
			ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight);
		}
		else
		{
			// Align width to the right side
			ImGui::SetNextItemWidth(-1.0f);
		}
		// Texture browser
		if (ImGui::BeginCombo("##Texture2D", m_Buffer ? m_Buffer->GetFileName().c_str() : nullptr))
		{
			bool bIsBufferChanged = false;
			// Pop up file browser to select a texture from disk
			if (ImGui::Selectable("Browse texture..."))
			{
				auto filePath = FileDialogs::OpenFile(AssetType::Texture);
				if (filePath)
				{
					// Add selected texture to the library
					Ref<Texture2D> loadedTexture = library.GetOrLoad(*filePath);
					bIsBufferChanged = loadedTexture != m_Buffer;
					if (bIsBufferChanged)
					{
						m_Buffer = loadedTexture;
						SetValueToData();
						//InvokePostDataValueChangeCallback();
					}
				}
			}

			ImGui::Separator();

			// List all loaded textures from Texture2DLibrary
			for (const auto& [path, texture] : library.GetTexturesMap())
			{
				// Push texture path as id
				ImGui::PushID(texture->GetPath().c_str());
				{
					constexpr float textureThumbnailWidth = 30.0f;
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
						bIsBufferChanged = texture != m_Buffer;
						if (bIsBufferChanged)
						{
							m_Buffer = texture;
							SetValueToData();
							//InvokePostDataValueChangeCallback();
						}
					}
				}
				ImGui::PopID();
			}

			ImGui::EndCombo();
		}

		// Display texture path tooltip for current selection
		if (m_Buffer && ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", m_Buffer->GetPath().c_str());
		}

		PostDraw();
	}

	bool ContainerWidget::PreDraw(const entt::meta_any& compInstance, int32_t elementIndex)
	{
		m_DataSpec.Update(compInstance, elementIndex);

		const auto size = m_DataSpec.GetContainerSize();
		const ImGuiTreeNodeFlags flags = size > 0 ? DefaultContainerDataTreeNodeFlags : EmptyContainerDataTreeNodeFlags;

		// Data name
		bool bIsDataTreeExpanded = ImGui::TreeNodeEx(m_DataSpec.DataName, flags);
		// Data tooltip
		ShowPropertyTooltip(m_DataSpec.Data);
		// Switch to the right column
		ImGui::TableNextColumn();

		// Add and clear buttons
		DrawContainerOperationWidget();
		// Switch to the next row
		ImGui::TableNextColumn();

		return bIsDataTreeExpanded;
	}

	void ContainerWidget::PostDraw()
	{
		ImGui::TreePop();
	}

	SequenceContainerWidget::SequenceContainerWidget(const DataSpec& dataSpec)
	{
		Init(dataSpec);
		DataSpec elementDataSpec{ dataSpec.Data, dataSpec.ComponentInstance, true };
		m_ElementWidgetTemplate = ConstructBasicDataWidget(elementDataSpec);
	}

	void SequenceContainerWidget::Draw(const entt::meta_any& compInstance, int32_t elementIndex)
	{
		if (!m_ElementWidgetTemplate) return;

		if (!PreDraw(compInstance, elementIndex)) return;

		uint32_t i = 0;
		for (auto it = m_DataSpec.SeqView.begin(); it != m_DataSpec.SeqView.end();)
		{
			auto element = *it;

			// Data index
			char nameBuffer[16];
			_itoa_s(i, nameBuffer, 10);
			bool bIsTreeExpanded = ImGui::TreeNodeEx(nameBuffer, DefaultDataTreeNodeFlags);
			// Switch to the right column
			ImGui::TableNextColumn();
			// Push data index as id
			ImGui::PushID(i);
			{
				// Make sure element widget + dropdown button can reach desired size
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 5.0f;
				ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight);
				// Draw element widget
				m_ElementWidgetTemplate->Draw(compInstance, i);
				ImGui::SameLine();
				// Insert and erase buttons
				DrawContainerElementOperationWidget(it);
				// Switch to the next row
				ImGui::TableNextColumn();
			}
			ImGui::PopID();

			if (it != m_DataSpec.SeqView.end())
			{
				++it, ++i;
			}
		}

		PostDraw();
	}

	void SequenceContainerWidget::DrawContainerOperationWidget()
	{
		const auto seqSize = m_DataSpec.SeqView.size();
		ImGui::Text("%d elements", seqSize);
		ImGui::SameLine();
		if (ImGui::BeginCombo("##SequenceContainerOperation", nullptr, ImGuiComboFlags_NoPreview))
		{
			if (ImGui::Selectable("Add"))
			{
				InsertValue(m_DataSpec.SeqView.end());
			}
			if (ImGui::Selectable("Clear"))
			{
				if (seqSize > 0 && m_DataSpec.SeqView.clear())
				{
					//InvokePostDataValueEditChangeCallback(data, {});
				}
			}

			ImGui::EndCombo();
		}
	}

	void SequenceContainerWidget::DrawContainerElementOperationWidget(entt::meta_sequence_container::iterator& it)
	{
		if (ImGui::BeginCombo("##SequenceContainerElementOperation", nullptr, ImGuiComboFlags_NoPreview))
		{
			if (ImGui::Selectable("Insert"))
			{
				it = InsertValue(it);
			}
			if (ImGui::Selectable("Erase"))
			{
				it = EraseValue(it);
			}

			ImGui::EndCombo();
		}
	}

	entt::meta_sequence_container::iterator SequenceContainerWidget::InsertValue(entt::meta_sequence_container::iterator it)
	{
		const auto elementType = m_DataSpec.SeqView.value_type();
		auto [retIt, res] = m_DataSpec.SeqView.insert(it, elementType.construct()); // Construct the pre-registered type with default value
		if (res)
		{
			//InvokePostDataValueEditChangeCallback(data, {});
			return retIt;
		}
		else
		{
			auto dataName = GetMetaObjectDisplayName(m_DataSpec.Data);
			ZE_CORE_ASSERT(false, "Failed to insert with data: '{0}'! Please check if its type is properly registered.", *dataName);
		}

		return {};
	}

	entt::meta_sequence_container::iterator SequenceContainerWidget::EraseValue(entt::meta_sequence_container::iterator it)
	{
		auto [retIt, res] = m_DataSpec.SeqView.erase(it);
		if (res)
		{
			//InvokePostDataValueEditChangeCallback(data, {});
			return retIt;
		}
		else
		{
			auto dataName = GetMetaObjectDisplayName(m_DataSpec.Data);
			ZE_CORE_ERROR("Failed to erase with data: {0}!", *dataName);
		}
		
		return {};
	}

	AssociativeContainerWidget::AssociativeContainerWidget(const DataSpec& dataSpec)
	{
		Init(dataSpec);
	}

	void AssociativeContainerWidget::Draw(const entt::meta_any& compInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, elementIndex)) return;



		PostDraw();
	}

	void AssociativeContainerWidget::DrawContainerOperationWidget()
	{
		
	}

	void AssociativeContainerWidget::DrawContainerElementOperationWidget(entt::meta_associative_container::iterator& it)
	{

	}

	entt::meta_associative_container::iterator AssociativeContainerWidget::InsertValue(entt::meta_associative_container::iterator it)
	{
		return {};
	}

	entt::meta_associative_container::iterator AssociativeContainerWidget::EraseValue(entt::meta_associative_container::iterator it)
	{
		return {};
	}

}
