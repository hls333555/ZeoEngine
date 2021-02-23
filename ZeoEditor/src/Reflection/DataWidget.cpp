#include "Reflection/DataWidget.h"

#include <misc/cpp/imgui_stdlib.h>

#include "Engine/Utils/PlatformUtils.h"
#include "Panels/DataInspectorPanel.h"
#include "Dockspaces/EditorDockspace.h"

namespace ZeoEngine {

	uint32_t GetAggregatedDataID(entt::meta_data data)
	{
		return ImGui::GetCurrentWindow()->GetID(data.id());
	}

	Ref<DataWidget> ConstructBasicDataWidget(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		const bool bIsSeqElement = dataSpec.bIsSeqElement;
		const auto type = dataSpec.GetType();
		switch (EvaluateMetaType(type))
		{
		case BasicMetaType::STRUCT:
			return CreateRef<StructWidget>(dataSpec, contextPanel);
		case BasicMetaType::SEQCON:
			if (bIsSeqElement)
			{
				ZE_CORE_ERROR("Container nesting is not supported!");
				return {};
			}
			return CreateRef<SequenceContainerWidget>(dataSpec, contextPanel);
		case BasicMetaType::ASSCON:
			if (bIsSeqElement)
			{
				ZE_CORE_ERROR("Container nesting is not supported!");
				return {};
			}
			return CreateRef<AssociativeContainerWidget>(dataSpec, contextPanel);
		case BasicMetaType::BOOL:
			return CreateRef<BoolDataWidget>(dataSpec, contextPanel);
		case BasicMetaType::I8:
			return CreateRef<ScalarNDataWidget<int8_t>>(dataSpec, contextPanel, ImGuiDataType_S8, static_cast<int8_t>(INT8_MIN), static_cast<int8_t>(INT8_MAX), "%hhd");
		case BasicMetaType::I32:
			return CreateRef<ScalarNDataWidget<int32_t>>(dataSpec, contextPanel, ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d");
		case BasicMetaType::I64:
			return CreateRef<ScalarNDataWidget<int64_t>>(dataSpec, contextPanel, ImGuiDataType_S64, INT64_MIN, INT64_MAX, "%lld");
		case BasicMetaType::UI8:
			return CreateRef<ScalarNDataWidget<uint8_t>>(dataSpec, contextPanel, ImGuiDataType_U8, 0ui8, UINT8_MAX, "%hhu");
		case BasicMetaType::UI32:
			return CreateRef<ScalarNDataWidget<uint32_t>>(dataSpec, contextPanel, ImGuiDataType_U32, 0ui32, UINT32_MAX, "%u");
		case BasicMetaType::UI64:
			return CreateRef<ScalarNDataWidget<uint64_t>>(dataSpec, contextPanel, ImGuiDataType_U64, 0ui64, UINT64_MAX, "%llu");
		case BasicMetaType::FLOAT:
			return CreateRef<ScalarNDataWidget<float>>(dataSpec, contextPanel, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		case BasicMetaType::DOUBLE:
			return CreateRef<ScalarNDataWidget<double>>(dataSpec, contextPanel, ImGuiDataType_Double, -DBL_MAX, DBL_MAX, "%.3lf");
		case BasicMetaType::ENUM:
			return CreateRef<EnumDataWidget>(dataSpec, contextPanel);
		case BasicMetaType::STRING:
			return CreateRef<StringDataWidget>(dataSpec, contextPanel);
		case BasicMetaType::VEC2:
			return CreateRef<ScalarNDataWidget<glm::vec2, 2, float>>(dataSpec, contextPanel, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		case BasicMetaType::VEC3:
			return CreateRef<ScalarNDataWidget<glm::vec3, 3, float>>(dataSpec, contextPanel, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
		case BasicMetaType::VEC4:
			return CreateRef<ColorDataWidget>(dataSpec, contextPanel);
		case BasicMetaType::TEXTURE:
			return CreateRef<Texture2DDataWidget>(dataSpec, contextPanel);
		case BasicMetaType::PARTICLE:
			return CreateRef<ParticleTemplateDataWidget>(dataSpec, contextPanel);
		}

		return {};
	}

	void DataWidget::Init(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		m_DataSpec = dataSpec;
		m_ContextPanel = contextPanel;
		if (!m_DataSpec.bIsSeqElement)
		{
			Draw(m_DataSpec.ComponentInstance, m_DataSpec.ParentInstance);
		}
	}
	// TODO:
	void DataWidget::InvokeOnDataValueEditChangeCallback(entt::meta_data data, std::any oldValue)
	{
		ZE_TRACE("Value changed during edit!");
		Component* comp = m_DataSpec.ComponentInstance.try_cast<Component>();
		ZE_CORE_ASSERT(comp);
		comp->OnDataValueEditChange(data.id(), oldValue);
	}

	void DataWidget::InvokePostDataValueEditChangeCallback(entt::meta_data data, std::any oldValue)
	{
		ZE_TRACE("Value changed after edit!");
		Component* comp = m_DataSpec.ComponentInstance.try_cast<Component>();
		ZE_CORE_ASSERT(comp);
		comp->PostDataValueEditChange(data.id(), oldValue);
	}

	BoolDataWidget::BoolDataWidget(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		Init(dataSpec, contextPanel);
	}

	void BoolDataWidget::Draw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, parentInstance, elementIndex)) return;

		if (ImGui::Checkbox("##Bool", &m_Buffer))
		{
			SetValueToData();
		}

		PostDraw();
	}

	EnumDataWidget::EnumDataWidget(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		Init(dataSpec, contextPanel);
		InitEnumDatas();
	}

	void EnumDataWidget::Draw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, parentInstance, elementIndex)) return;

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
							Reflection::SetEnumValueForSeq(m_DataSpec.SeqView[m_DataSpec.ElementIndex], m_Buffer);
							InvokePostDataValueEditChangeCallback(m_DataSpec.Data, m_OldBuffer);
						}
						else
						{
							SetValueToData();
						}
						
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

	StringDataWidget::StringDataWidget(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		Init(dataSpec, contextPanel);
	}

	void StringDataWidget::Draw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, parentInstance, elementIndex)) return;

		ImGui::InputText("##String", &m_Buffer, ImGuiInputTextFlags_AutoSelectAll);
		// For tabbing (we must force set value back in this case or the buffer will be reset on the next draw)
		if (ImGui::IsKeyPressed(Key::Tab) && ImGui::GetFocusID() == ImGui::GetItemID())
		{
			SetValueToData();
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
			}
		}

		PostDraw();
	}

	ColorDataWidget::ColorDataWidget(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		Init(dataSpec, contextPanel);
	}

	void ColorDataWidget::Draw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, parentInstance, elementIndex)) return;

		bool bChanged = ImGui::ColorEdit4("", glm::value_ptr(m_Buffer));
		// For dragging
		if (bChanged && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			SetValueToData(false);
		}
		// For tabbing (we must force set value back in this case or the buffer will be reset on the next draw)
		if (ImGui::IsKeyPressed(Key::Tab) && ImGui::GetFocusID() == ImGui::GetItemID())
		{
			if (IsBufferChanged())
			{
				SetValueToData();
			}
		}
		// For multi-component widget, tabbing will switch to the next component, so we must handle deactivation to apply cache first
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (IsBufferChanged())
			{
				SetValueToData();
			}
		}
		if (ImGui::IsItemActivated())
		{
			UpdateBuffer();
		}

		PostDraw();
	}

	Texture2DDataWidget::Texture2DDataWidget(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		Init(dataSpec, contextPanel);
	}

	void Texture2DDataWidget::Draw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, parentInstance, elementIndex)) return;

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

	ParticleTemplateDataWidget::ParticleTemplateDataWidget(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		Init(dataSpec, contextPanel);
	}

	void ParticleTemplateDataWidget::Draw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, parentInstance, elementIndex)) return;

		ParticleLibrary& library = ParticleLibrary::Get();
		Texture2DLibrary& texture2DLib = Texture2DLibrary::Get();
		auto backgroundTexture = texture2DLib.Get("assets/textures/Checkerboard_Alpha.png");
		// Particle template preview
		{
			constexpr float pTemplatePreviewWidth = 75.0f;
			// Draw checkerboard texture as background first
			ImGui::GetWindowDrawList()->AddImage(backgroundTexture->GetTexture(),
				ImGui::GetCursorScreenPos(),
				{ ImGui::GetCursorScreenPos().x + pTemplatePreviewWidth, ImGui::GetCursorScreenPos().y + pTemplatePreviewWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f });
			// Draw preview thumbnail on top of that
			auto thumbnailTexture = m_Buffer && m_Buffer->PreviewThumbnail ? m_Buffer->PreviewThumbnail : backgroundTexture;
			ImGui::Image(thumbnailTexture->GetTexture(),
				{ pTemplatePreviewWidth, pTemplatePreviewWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f },
				m_Buffer ? ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f } : ImVec4{ 1.0f, 1.0f, 1.0f, 0.0f });
			// Double-click to open the particle editor only when particle template is valid
			if (m_Buffer && ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Double-click to open the particle editor");
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					EditorDockspace* editor = m_ContextPanel->GetContext()->OpenEditor(EditorDockspaceType::Particle_Editor);
					editor->GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([&](auto& pspc)
					{
						pspc.SetTemplate(m_Buffer);
					});
				}
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

		// Particle template browser
		if (ImGui::BeginCombo("##ParticleTemplate", m_Buffer ? m_Buffer->GetName().c_str() : nullptr))
		{
			bool bIsBufferChanged = false;
			// Pop up file browser to select a particle template from disk
			if (ImGui::Selectable("Browse particle template..."))
			{
				auto filePath = FileDialogs::OpenFile(AssetType::ParticleTemplate);
				if (filePath)
				{
					// Add selected particle template to the library
					Ref<ParticleTemplate> loadedTemplate = library.GetOrLoad(*filePath);
					bIsBufferChanged = loadedTemplate != m_Buffer;
					if (bIsBufferChanged)
					{
						m_Buffer = loadedTemplate;
						SetValueToData();
					}
				}
			}

			ImGui::Separator();

			// List all loaded templates from ParticleLibrary
			for (const auto& [path, pTemplate] : library.GetParticleTemplatesMap())
			{
				// Push particle template path as id
				ImGui::PushID(pTemplate->GetPath().c_str());
				{
					const float pTemplateThumbnailWidth = 30.0f;
					bool bIsSelected = ImGui::Selectable("##ParticleTemplateDropdownThumbnail", false, 0, ImVec2(0.0f, pTemplateThumbnailWidth));
					// Display particle template path tooltip for drop-down item
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("%s", pTemplate->GetPath().c_str());
					}

					ImGui::SameLine();

					// Draw particle template thumbnail
					auto thumbnailTexture = pTemplate && pTemplate->PreviewThumbnail ? pTemplate->PreviewThumbnail : backgroundTexture;
					ImGui::Image(thumbnailTexture->GetTexture(),
						ImVec2(pTemplateThumbnailWidth, pTemplateThumbnailWidth),
						ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

					ImGui::SameLine();

					// Display particle template name
					ImGui::Text(pTemplate->GetName().c_str());
					if (bIsSelected)
					{
						bIsBufferChanged = pTemplate != m_Buffer;
						if (bIsBufferChanged)
						{
							m_Buffer = pTemplate;
							SetValueToData();
						}
					}
				}
				ImGui::PopID();
			}

			ImGui::EndCombo();
		}

		// Display particle template path tooltip for current selection
		if (m_Buffer && ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", m_Buffer->GetPath().c_str());
		}

		PostDraw();
	}

	bool ContainerWidget::PreDraw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		m_DataSpec.Update(compInstance, parentInstance, elementIndex);

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

	SequenceContainerWidget::SequenceContainerWidget(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		Init(dataSpec, contextPanel);
		DataSpec elementDataSpec{ dataSpec.Data, dataSpec.ComponentInstance, dataSpec.ParentInstance, false, true };
		m_ElementWidgetTemplate = ConstructBasicDataWidget(elementDataSpec, m_ContextPanel);
	}

	void SequenceContainerWidget::Draw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		if (!m_ElementWidgetTemplate) return;

		if (!PreDraw(compInstance, parentInstance, elementIndex)) return;

		uint32_t i = 0;
		for (auto it = m_DataSpec.SeqView.begin(); it != m_DataSpec.SeqView.end();)
		{
			char nameBuffer[16];
			_itoa_s(i, nameBuffer, 10);
			bool bIsElementStruct = DoesPropExist(PropertyType::Struct, m_DataSpec.SeqView.value_type());
			ImGuiTreeNodeFlags flags = bIsElementStruct ? DefaultStructDataTreeNodeFlags : DefaultDataTreeNodeFlags;
			// Data index
			bool bIsTreeExpanded = ImGui::TreeNodeEx(nameBuffer, flags);
			// Switch to the right column
			ImGui::TableNextColumn();
			// Push data index as id
			ImGui::PushID(i);
			{
				if (bIsElementStruct)
				{
					// Insert and erase buttons
					DrawContainerElementOperationWidget(it);
					// Switch to the next row
					ImGui::TableNextColumn();
					if (bIsTreeExpanded)
					{
						// If we erased the last element just now, the widget should not be drawn
						if (it != m_DataSpec.SeqView.end())
						{
							// Draw element widget
							m_ElementWidgetTemplate->Draw(compInstance, compInstance, i);
						}

						ImGui::TreePop();
					}
				}
				else
				{
					// Make sure element widget + dropdown button can reach desired size
					ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
					float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 5.0f;
					ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight);
					// Draw element widget
					m_ElementWidgetTemplate->Draw(compInstance, compInstance, i);
					ImGui::SameLine();
					// Insert and erase buttons
					DrawContainerElementOperationWidget(it);
					// Switch to the next row
					ImGui::TableNextColumn();
				}
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
					InvokePostDataValueEditChangeCallback(m_DataSpec.Data, {});
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
			InvokePostDataValueEditChangeCallback(m_DataSpec.Data, {});
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
			InvokePostDataValueEditChangeCallback(m_DataSpec.Data, {});
			return retIt;
		}
		else
		{
			auto dataName = GetMetaObjectDisplayName(m_DataSpec.Data);
			ZE_CORE_ERROR("Failed to erase with data: {0}!", *dataName);
		}
		
		return {};
	}

	AssociativeContainerWidget::AssociativeContainerWidget(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		Init(dataSpec, contextPanel);
	}

	void AssociativeContainerWidget::Draw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, parentInstance, elementIndex)) return;



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

	StructWidget::StructWidget(const DataSpec& dataSpec, DataInspectorPanel* contextPanel)
	{
		Init(dataSpec, contextPanel);
	}

	bool StructWidget::PreDraw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		m_DataSpec.Update(compInstance, parentInstance, elementIndex);

		// Data name
		bool bIsDataTreeExpanded = ImGui::TreeNodeEx(m_DataSpec.DataName, DefaultStructDataTreeNodeFlags);
		// Data tooltip
		ShowPropertyTooltip(m_DataSpec.Data);
		// Switch to the right column
		ImGui::TableNextColumn();
		// Switch to the next row
		ImGui::TableNextColumn();

		return bIsDataTreeExpanded;
	}

	void StructWidget::Draw(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, parentInstance, elementIndex)) return;

		const auto structType = m_DataSpec.GetType();
		auto structInstance = m_DataSpec.GetValue();

		// Preprocess subdatas if needed
		if (m_bIsPreprocessedSubdatasDirty)
		{
			const auto compName = GetMetaObjectDisplayName(compInstance.type());
			ZE_CORE_TRACE("Sorting subdatas on {0} of '{1}'", m_DataSpec.DataName, *compName);
			PreprocessStruct(structType);
			// TODO:
			m_bIsPreprocessedSubdatasDirty = false;
		}

		std::vector<entt::meta_data> visibleSubdatas;
		for (const auto subDataId : m_PreprocessedSubdatas)
		{
			entt::meta_data subdata = structType.data(subDataId);
			
			// TODO:
			if (/*!ShouldHideData(subdata, structInstance)*/true)
			{
				visibleSubdatas.push_back(subdata);
			}
		}
		for (const auto subdata : visibleSubdatas)
		{
			// Push subdata id
			ImGui::PushID(subdata.id());
			{
				// Draw widget based on data type
				DrawSubdataWidget(subdata, structInstance);
			}
			ImGui::PopID();
		}

		PostDraw();
	}

	void StructWidget::PostDraw()
	{
		ImGui::TreePop();
	}

	void StructWidget::PreprocessStruct(entt::meta_type structType)
	{
		for (const auto subdata : structType.data())
		{
			PreprocessSubdata(subdata);
		}
	}

	void StructWidget::PreprocessSubdata(entt::meta_data subdata)
	{
		// Reverse subdata display order
		m_PreprocessedSubdatas.push_front(subdata.id());
	}

	void StructWidget::DrawSubdataWidget(entt::meta_data subdata, const entt::meta_any& structInstance)
	{
		uint32_t aggregatedDataId = GetAggregatedDataID(subdata);
		if (m_SubdataWidgets.find(aggregatedDataId) != m_SubdataWidgets.cend())
		{
			if (m_SubdataWidgets[aggregatedDataId])
			{
				m_SubdataWidgets[aggregatedDataId]->Draw(m_DataSpec.ComponentInstance, structInstance);
			}
		}
		else
		{
			DataSpec dataSpec{ subdata, m_DataSpec.ComponentInstance, structInstance, true, false };
			m_SubdataWidgets[aggregatedDataId] = ConstructBasicDataWidget(dataSpec, m_ContextPanel);
		}
	}

}
