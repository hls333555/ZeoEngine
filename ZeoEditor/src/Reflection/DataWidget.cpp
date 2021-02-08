#include "Reflection/DataWidget.h"

#include <misc/cpp/imgui_stdlib.h>

#include "Engine/Utils/PlatformUtils.h"

namespace ZeoEngine {

	BoolDataWidget::BoolDataWidget(const DataSpec& dataSpec)
	{
		Init(dataSpec);
	}

	void BoolDataWidget::Draw(const entt::meta_any& compInstance)
	{
		if (!PreDraw(compInstance)) return;

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

	void EnumDataWidget::Draw(const entt::meta_any& compInstance)
	{
		if (!PreDraw(compInstance)) return;

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
						SetValueToData();
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
		for (const auto enumData : m_Buffer.type().data())
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

	void StringDataWidget::Draw(const entt::meta_any& compInstance)
	{
		if (!PreDraw(compInstance)) return;

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

	void ColorDataWidget::Draw(const entt::meta_any& compInstance)
	{
		if (!PreDraw(compInstance)) return;

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

	void Texture2DDataWidget::Draw(const entt::meta_any& compInstance)
	{
		if (!PreDraw(compInstance)) return;

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

		// Align width to the right side
		ImGui::SetNextItemWidth(-1.0f);
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

	bool ContainerWidget::PreDraw(const entt::meta_any& compInstance)
	{
		m_DataSpec.ComponentInstance = compInstance;

		const auto size = m_DataSpec.GetValue().as_sequence_container().size();
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
	}

	void SequenceContainerWidget::Draw(const entt::meta_any& compInstance)
	{
		if (!PreDraw(compInstance)) return;

		uint32_t i = 0;
		for (auto it = m_SeqView.begin(); it != m_SeqView.end();)
		{
			auto element = *it;

			// Data index
			bool bIsTreeExpanded = ImGui::TreeNodeEx(std::to_string(i).c_str(), DefaultDataTreeNodeFlags);
			// Switch to the right column
			ImGui::TableNextColumn();
			// Push data index as id
			ImGui::PushID(i);
			{
				// Make sure element widget + dropdown button can reach desired size
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight);
				// TODO: Draw element widgets

				ImGui::SameLine();
				// Insert and erase buttons
				DrawContainerElementOperationWidget(it);
				// Switch to the next row
				ImGui::TableNextColumn();
			}
			ImGui::PopID();

			if (it != m_SeqView.end())
			{
				++it, ++i;
			}
		}

		PostDraw();
	}

	void SequenceContainerWidget::DrawContainerOperationWidget()
	{
		m_SeqView = m_DataSpec.GetValue().as_sequence_container();

		ImGui::Text("%d elements", m_SeqView.size());
		ImGui::SameLine();
		if (ImGui::BeginCombo("##SequenceContainerOperation", nullptr, ImGuiComboFlags_NoPreview))
		{
			if (ImGui::Selectable("Add"))
			{
				InsertValue(m_SeqView.end());
			}
			if (ImGui::Selectable("Clear"))
			{
				if (m_SeqView.size() > 0 && m_SeqView.clear())
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
		const auto elementType = m_SeqView.value_type();
		auto [retIt, res] = m_SeqView.insert(it, elementType.construct()); // Construct the pre-registered type with default value
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
		auto [retIt, res] = m_SeqView.erase(it);
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

	void AssociativeContainerWidget::Draw(const entt::meta_any& compInstance)
	{
		if (!PreDraw(compInstance)) return;



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
