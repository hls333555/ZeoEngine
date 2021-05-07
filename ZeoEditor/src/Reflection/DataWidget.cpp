#include "Reflection/DataWidget.h"

#include <misc/cpp/imgui_stdlib.h>
#include <IconsFontAwesome5.h>

#include "Engine/GameFramework/ComponentHelpers.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Core/WindowManager.h"
#include "Dockspaces/DockspaceBase.h"

namespace ZeoEngine {

	uint32_t GetAggregatedDataID(entt::meta_data data)
	{
		return ImGui::GetCurrentWindow()->GetID(data.id());
	}

	Ref<DataWidget> ConstructBasicDataWidget(DataSpec& dataSpec, entt::meta_type type, bool bIsTest)
	{
		const bool bIsSeqElement = dataSpec.bIsSeqElement;
		switch (EvaluateMetaType(type))
		{
			case BasicMetaType::STRUCT:
				return CreateRef<StructWidget>(dataSpec, bIsTest);
			case BasicMetaType::SEQCON:
				if (bIsSeqElement)
				{
					ZE_CORE_ERROR("Container nesting is not supported!");
					return {};
				}
				return CreateRef<SequenceContainerWidget>(dataSpec, bIsTest);
			case BasicMetaType::ASSCON:
				if (bIsSeqElement)
				{
					ZE_CORE_ERROR("Container nesting is not supported!");
					return {};
				}
				return CreateRef<AssociativeContainerWidget>(dataSpec, bIsTest);
			case BasicMetaType::BOOL:
				return CreateRef<BoolDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::I8:
				return CreateRef<ScalarNDataWidget<int8_t>>(dataSpec, bIsTest, ImGuiDataType_S8, static_cast<int8_t>(INT8_MIN), static_cast<int8_t>(INT8_MAX), "%hhd");
			case BasicMetaType::I32:
				return CreateRef<ScalarNDataWidget<int32_t>>(dataSpec, bIsTest, ImGuiDataType_S32, INT32_MIN, INT32_MAX, "%d");
			case BasicMetaType::I64:
				return CreateRef<ScalarNDataWidget<int64_t>>(dataSpec, bIsTest, ImGuiDataType_S64, INT64_MIN, INT64_MAX, "%lld");
			case BasicMetaType::UI8:
				return CreateRef<ScalarNDataWidget<uint8_t>>(dataSpec, bIsTest, ImGuiDataType_U8, 0ui8, UINT8_MAX, "%hhu");
			case BasicMetaType::UI32:
				return CreateRef<ScalarNDataWidget<uint32_t>>(dataSpec, bIsTest, ImGuiDataType_U32, 0ui32, UINT32_MAX, "%u");
			case BasicMetaType::UI64:
				return CreateRef<ScalarNDataWidget<uint64_t>>(dataSpec, bIsTest, ImGuiDataType_U64, 0ui64, UINT64_MAX, "%llu");
			case BasicMetaType::FLOAT:
				return CreateRef<ScalarNDataWidget<float>>(dataSpec, bIsTest, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
			case BasicMetaType::DOUBLE:
				return CreateRef<ScalarNDataWidget<double>>(dataSpec, bIsTest, ImGuiDataType_Double, -DBL_MAX, DBL_MAX, "%.3lf");
			case BasicMetaType::ENUM:
				return CreateRef<EnumDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::STRING:
				return CreateRef<StringDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::VEC2:
				return CreateRef<ScalarNDataWidget<glm::vec2, 2, float>>(dataSpec, bIsTest, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
			case BasicMetaType::VEC3:
				return CreateRef<ScalarNDataWidget<glm::vec3, 3, float>>(dataSpec, bIsTest, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.2f");
			case BasicMetaType::VEC4:
				return CreateRef<ColorDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::TEXTURE:
				return CreateRef<Texture2DDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::PARTICLE:
				return CreateRef<ParticleTemplateDataWidget>(dataSpec, bIsTest);
		}

		return {};
	}

#ifndef DOCTEST_CONFIG_DISABLE
	void DataWidget::Test(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
	{
		char dataStr[64];
		strcpy_s(dataStr, m_DataSpec.DataName);
		char suffix[] = " (container element)";
		if (m_DataSpec.bIsSeqElement)
		{
			strcat_s(dataStr, suffix);
		}
		INFO(dataStr);
		TestImpl(reg, entity, dataStack, elementIndex);
	}
#endif

	void DataWidget::Init(DataSpec& dataSpec, bool bIsTest)
	{
		m_DataSpec = dataSpec;
		m_bIsTest = bIsTest;
		if (!bIsTest && !m_DataSpec.bIsSeqElement)
		{
			Draw(m_DataSpec.ComponentInstance, m_DataSpec.Instance);
		}
	}

	void DataWidget::InvokeOnDataValueEditChangeCallback(entt::meta_data data, std::any oldValue)
	{
		if (m_bIsTest) return;

		ZE_TRACE("Value changed during edit!");
		IComponent* comp = m_DataSpec.ComponentInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		if (comp->ComponentHelper)
		{
			comp->ComponentHelper->OnComponentDataValueEditChange(data.id(), oldValue);
		}
	}

	void DataWidget::InvokePostDataValueEditChangeCallback(entt::meta_data data, std::any oldValue)
	{
		if (m_bIsTest) return;

		ZE_TRACE("Value changed after edit!");
		IComponent* comp = m_DataSpec.ComponentInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		if (comp->ComponentHelper)
		{
			comp->ComponentHelper->PostComponentDataValueEditChange(data.id(), oldValue);
		}
	}

	BoolDataWidget::BoolDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void BoolDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		if (!PreDraw(compInstance, instance)) return;

		if (ImGui::Checkbox("##Bool", &m_Buffer))
		{
			SetValueToData();
		}

		PostDraw();
	}

#ifndef DOCTEST_CONFIG_DISABLE
	void BoolDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
	{
		m_Buffer = true;
		SetValueToData();
		CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
	}
#endif

	EnumDataWidget::EnumDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void EnumDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		if (!PreDraw(compInstance, instance)) return;

		if (m_EnumDatas.empty())
		{
			InitEnumDatas();
		}

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
							Reflection::SetEnumValueForSeq(instance, m_Buffer);
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
		const auto type = m_DataSpec.GetType();
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

#ifndef DOCTEST_CONFIG_DISABLE
	void EnumDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
{
		if (m_EnumDatas.empty())
		{
			InitEnumDatas();
		}

		REQUIRE(m_EnumDatas.size() > 1);
		uint32_t i = 0;
		for (const auto enumData : m_EnumDatas)
		{
			if (i == 1)
			{
				m_Buffer = enumData.get({});
				if (m_DataSpec.bIsSeqElement)
				{
					Reflection::SetEnumValueForSeq(m_DataSpec.Instance, m_Buffer);
				}
				else
				{
					SetValueToData();
				}
				CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
				break;
			}
			++i;
		}
	}
#endif

	StringDataWidget::StringDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void StringDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		if (!PreDraw(compInstance, instance)) return;

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

#ifndef DOCTEST_CONFIG_DISABLE
	void StringDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
{
		m_Buffer = std::string("Test");
		SetValueToData();
		CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
	}
#endif

	ColorDataWidget::ColorDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void ColorDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		if (!PreDraw(compInstance, instance)) return;

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

#ifndef DOCTEST_CONFIG_DISABLE
	void ColorDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
{
		m_Buffer = glm::vec4(10.0f);
		SetValueToData();
		CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
	}
#endif

	static float GetDropdownWidth()
	{
		return ImGui::GetFontSize() + ImGui::GetFramePadding().y * 5.0f;
	}

	Texture2DDataWidget::Texture2DDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void Texture2DDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		if (!PreDraw(compInstance, instance)) return;

		Texture2DLibrary& library = Texture2DLibrary::Get();
		// Texture preview
		{
			auto backgroundTexture = library.GetAsset("assets/textures/Checkerboard_Alpha.png");
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
				ImGui::SetTooltipWithPadding("Resolution: %dx%d\nHas alpha: %s", m_Buffer->GetWidth(), m_Buffer->GetHeight(), m_Buffer->HasAlpha() ? "true" : "false");
			}
		}

		ImGui::SameLine();

		if (m_DataSpec.bIsSeqElement)
		{
			// Make sure browser widget + dropdown button can reach desired size
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - GetDropdownWidth());
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

			// Clear current selection
			if (ImGui::Selectable("Clear"))
			{
				bIsBufferChanged = static_cast<bool>(m_Buffer);
				if (bIsBufferChanged)
				{
					m_Buffer.reset();
					SetValueToData();
				}
			}

			ImGui::Separator();

			// Pop up file browser to select a texture from disk
			if (ImGui::Selectable("Browse texture..."))
			{
				auto filePath = FileDialogs::OpenFile(AssetType::Texture);
				if (filePath)
				{
					// Add selected texture to the library
					Ref<Texture2D> loadedTexture = library.GetOrLoadAsset(*filePath);
					bIsBufferChanged = loadedTexture != m_Buffer;
					if (bIsBufferChanged)
					{
						m_Buffer = loadedTexture;
						SetValueToData();
					}
				}
			}

			ImGui::Separator();

			m_Filter.Draw("##Texture2DAssetFilter", "Search textures");
			
			// List all loaded textures from Texture2DLibrary
			for (const auto& [path, texture] : library.GetAssetsMap())
			{
				if (!m_Filter.IsActive() || m_Filter.IsActive() && m_Filter.PassFilter(texture->GetFileName().c_str()))
				{
					// Push texture path as id
					ImGui::PushID(path.c_str());
					{
						constexpr float textureThumbnailWidth = 30.0f;
						bool bIsSelected = ImGui::Selectable("##Texture2DDropdownThumbnail", false, 0, ImVec2(0.0f, textureThumbnailWidth));
						// Display texture path tooltip for drop-down item
						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltipWithPadding("%s", path.c_str());
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
			}

			ImGui::EndCombo();
		}

		// Display texture path tooltip for current selection
		if (m_Buffer && ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("%s", m_Buffer->GetPath().c_str());
		}

		PostDraw();
	}

#ifndef DOCTEST_CONFIG_DISABLE
	void Texture2DDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
{
		m_Buffer = Texture2DLibrary::Get().GetOrLoadAsset("assets/textures/Checkerboard_Alpha.png");
		SetValueToData();
		CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
	}
#endif

	ParticleTemplateDataWidget::ParticleTemplateDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void ParticleTemplateDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		if (!PreDraw(compInstance, instance)) return;

		ParticleLibrary& library = ParticleLibrary::Get();
		Texture2DLibrary& texture2DLib = Texture2DLibrary::Get();
		auto backgroundTexture = texture2DLib.GetAsset("assets/textures/Checkerboard_Alpha.png");
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
			
			// If particle template is set...
			if (m_Buffer)
			{
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Double-click to open the particle editor\nRight-click to open the context menu");
					// Double-click on the preview thumbnail to open the particle editor
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						DockspaceBase* editor = DockspaceManager::Get().ToggleDockspace(DockspaceType::ParticleEditor, true);
						editor->GetContextEntity().PatchComponent<ParticleSystemPreviewComponent>([&](auto& particlePreviewComp)
						{
							ParticleSystemInstance::Create(particlePreviewComp, m_Buffer);
						});
					}
				}

				// Right-click on the preview thumbnail to open the popup menu
				if (ImGui::BeginPopupContextItemWithPadding("ParticleTemplateOptiones"))
				{
					if (ImGui::MenuItem(ICON_FA_REDO "  Resimulate"))
					{
						m_DataSpec.ComponentInstance.cast<ParticleSystemComponent>().Instance->Resimulate();
					}

					ImGui::EndPopup();
				}
			}
			
		}

		ImGui::SameLine();

		if (m_DataSpec.bIsSeqElement)
		{
			// Make sure browser widget + dropdown button can reach desired size
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - GetDropdownWidth());
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

			// Clear current selection
			if (ImGui::Selectable("Clear"))
			{
				bIsBufferChanged = static_cast<bool>(m_Buffer);
				if (bIsBufferChanged)
				{
					m_Buffer.reset();
					SetValueToData();
				}
			}

			ImGui::Separator();

			// Pop up file browser to select a particle template from disk
			if (ImGui::Selectable("Browse particle template..."))
			{
				auto filePath = FileDialogs::OpenFile(AssetType::ParticleTemplate);
				if (filePath)
				{
					// Add selected particle template to the library
					Ref<ParticleTemplate> loadedTemplate = library.GetOrLoadAsset(*filePath);
					bIsBufferChanged = loadedTemplate != m_Buffer;
					if (bIsBufferChanged)
					{
						m_Buffer = loadedTemplate;
						SetValueToData();
					}
				}
			}

			ImGui::Separator();

			m_Filter.Draw("##ParticleTemplateAssetFilter", "Search particle templates");

			// List all loaded templates from ParticleLibrary
			for (const auto& [path, pTemplate] : library.GetAssetsMap())
			{
				if (!m_Filter.IsActive() || m_Filter.IsActive() && m_Filter.PassFilter(pTemplate->GetName().c_str()))
				{
					// Push particle template path as id
					ImGui::PushID(path.c_str());
					{
						const float pTemplateThumbnailWidth = 30.0f;
						bool bIsSelected = ImGui::Selectable("##ParticleTemplateDropdownThumbnail", false, 0, ImVec2(0.0f, pTemplateThumbnailWidth));
						// Display particle template path tooltip for drop-down item
						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltipWithPadding("%s", path.c_str());
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
			}

			ImGui::EndCombo();
		}

		// Display particle template path tooltip for current selection
		if (m_Buffer && ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("%s", m_Buffer->GetPath().c_str());
		}

		PostDraw();
	}

#ifndef DOCTEST_CONFIG_DISABLE
	void ParticleTemplateDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
{
		m_Buffer = ParticleLibrary::Get().GetOrLoadAsset("assets/particles/Test.zparticle");
		SetValueToData();
		CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
	}
#endif

	bool ContainerWidget::PreDraw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		m_DataSpec.Update(compInstance, instance);

		auto seqView = m_DataSpec.GetValue().as_sequence_container();
		const auto size = seqView.size();
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

	SequenceContainerWidget::SequenceContainerWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
		if (!bIsTest)
		{
			DataSpec elementDataSpec{ dataSpec.Data, dataSpec.ComponentInstance, dataSpec.Instance, false, true };
			auto seqView = m_DataSpec.GetValue().as_sequence_container();
			m_ElementWidgetTemplate = ConstructBasicDataWidget(elementDataSpec, seqView.value_type());
		}
	}

	void SequenceContainerWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		if (!m_ElementWidgetTemplate) return;

		if (!PreDraw(compInstance, instance)) return;

		auto seqView = m_DataSpec.GetValue().as_sequence_container();
		uint32_t i = 0;
		for (auto it = seqView.begin(); it != seqView.end();)
		{
			auto elementInstance = *it;

			ImGui::AlignTextToFramePadding();
			char indexNameBuffer[16];
			_itoa_s(i, indexNameBuffer, 10);
			bool bIsElementStruct = DoesPropExist(PropertyType::Struct, seqView.value_type());
			ImGuiTreeNodeFlags flags = bIsElementStruct ? DefaultStructDataTreeNodeFlags : DefaultDataTreeNodeFlags;
			// Data index
			bool bIsTreeExpanded = ImGui::TreeNodeEx(indexNameBuffer, flags);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Drag to re-arrange elements");
			}

			// Drag-drop operation
			{
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("DragSeqContainerIndex", &i, sizeof(uint32_t));
					ImGui::Text("Place it here");

					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DragSeqContainerIndex"))
					{
						ZE_CORE_ASSERT(payload->DataSize == sizeof(uint32_t));

						auto sourceIndex = *(const uint32_t*)payload->Data;
						auto targetIndex = i;
						bool bMoveDownward = targetIndex >= sourceIndex;

						// Insert to target location
						{
							auto sourceElement = seqView[sourceIndex];
							auto targetIt = it;
							auto [retIt, res] = seqView.insert(bMoveDownward ? ++targetIt : targetIt, sourceElement);
							ZE_CORE_ASSERT(res);
						}

						// Erase from source location
						{
							auto sourceIt = seqView.begin();
							for (uint32_t j = 0; j < sourceIndex; ++j)
							{
								++sourceIt;
							}
							auto [retIt, res] = seqView.erase(bMoveDownward ? sourceIt : ++sourceIt);
							ZE_CORE_ASSERT(res);
						}

						// Update iterator to last draw location
						{
							it = seqView.begin();
							for (uint32_t j = 0; j < i; ++j)
							{
								++it;
							}
						}

						// Update instance
						elementInstance = *it;
					}

					ImGui::EndDragDropTarget();
				}
			}

			// Switch to the right column
			ImGui::TableNextColumn();
			// Push data index as id
			ImGui::PushID(i);
			{
				if (bIsElementStruct)
				{
					// Insert and erase buttons
					DrawContainerElementOperationWidget(seqView, it);
					// Switch to the next row
					ImGui::TableNextColumn();
					if (bIsTreeExpanded)
					{
						// If we erased the last element just now, the widget should not be drawn
						if (it != seqView.end())
						{
							// Draw element widget
							m_ElementWidgetTemplate->Draw(compInstance, elementInstance);
						}

						ImGui::TreePop();
					}
				}
				else
				{
					// Make sure element widget + dropdown button can reach desired size
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - GetDropdownWidth());
					// Draw element widget
					m_ElementWidgetTemplate->Draw(compInstance, elementInstance);
					ImGui::SameLine();
					// Insert and erase buttons
					DrawContainerElementOperationWidget(seqView, it);
					// Switch to the next row
					ImGui::TableNextColumn();
				}
			}
			ImGui::PopID();

			if (it != seqView.end())
			{
				++it, ++i;
			}
		}

		PostDraw();
	}

#ifndef DOCTEST_CONFIG_DISABLE
	void SequenceContainerWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
	{
		auto seqView = m_DataSpec.GetValue().as_sequence_container();
		auto retIt = InsertValue(seqView, seqView.end());
		REQUIRE(seqView.size() == 1);
		auto elementInstance = *retIt; // This line is necessary!
		DataSpec elementDataSpec{ m_DataSpec.Data, m_DataSpec.ComponentInstance, elementInstance, false, true };
		auto elementWidgetTemplate = ConstructBasicDataWidget(elementDataSpec, seqView.value_type(), true);
		elementWidgetTemplate->Test(reg, entity, dataStack, 0);
		seqView.clear();
		REQUIRE(seqView.size() == 0);
	}
#endif

	void SequenceContainerWidget::DrawContainerOperationWidget()
	{
		auto seqView = m_DataSpec.GetValue().as_sequence_container();
		const auto seqSize = seqView.size();
		ImGui::Text("%d elements", seqSize);

		ImGui::SameLine();

		if (ImGui::TransparentSmallButton(ICON_FA_PLUS))
		{
			InsertValue(seqView, seqView.end());
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Add a element to the last");
		}

		ImGui::SameLine();

		if (ImGui::TransparentSmallButton(ICON_FA_TRASH))
		{
			if (seqSize > 0 && seqView.clear())
			{
				InvokePostDataValueEditChangeCallback(m_DataSpec.Data, {});
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Remove all elements");
		}
	}

	void SequenceContainerWidget::DrawContainerElementOperationWidget(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator& it)
	{
		if (ImGui::BeginCombo("##SequenceContainerElementOperation", nullptr, ImGuiComboFlags_NoPreview))
		{
			if (ImGui::Selectable("Insert"))
			{
				it = InsertValue(seqView, it);
			}
			if (ImGui::Selectable("Erase"))
			{
				it = EraseValue(seqView, it);
			}

			ImGui::EndCombo();
		}
	}

	entt::meta_sequence_container::iterator SequenceContainerWidget::InsertValue(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it)
	{
		const auto elementType = seqView.value_type();
		auto [retIt, res] = seqView.insert(it, elementType.construct()); // Construct the pre-registered type with default value
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

	entt::meta_sequence_container::iterator SequenceContainerWidget::EraseValue(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it)
	{
		auto [retIt, res] = seqView.erase(it);
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

	AssociativeContainerWidget::AssociativeContainerWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void AssociativeContainerWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		if (!PreDraw(compInstance, instance)) return;



		PostDraw();
	}

	void AssociativeContainerWidget::DrawContainerOperationWidget()
	{
		
	}

	void AssociativeContainerWidget::DrawContainerElementOperationWidget(entt::meta_associative_container& assView, entt::meta_associative_container::iterator& it)
	{

	}

	entt::meta_associative_container::iterator AssociativeContainerWidget::InsertValue(entt::meta_associative_container& assView, entt::meta_associative_container::iterator it)
	{
		return {};
	}

	entt::meta_associative_container::iterator AssociativeContainerWidget::EraseValue(entt::meta_associative_container& assView, entt::meta_associative_container::iterator it)
	{
		return {};
	}

	StructWidget::StructWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	bool StructWidget::PreDraw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		m_DataSpec.Update(compInstance, instance);

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

	void StructWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance)
	{
		if (!PreDraw(compInstance, instance)) return;

		const auto structType = m_DataSpec.GetType();
		auto structInstance = m_DataSpec.GetValue();

		// Preprocess subdatas if needed
		if (m_bIsPreprocessedSubdatasDirty)
		{
			const auto compName = GetMetaObjectDisplayName(compInstance.type());
			ZE_CORE_TRACE("Sorting subdatas on '{0}' of '{1}'", m_DataSpec.DataName, *compName);
			PreprocessStruct(structType);
			m_bIsPreprocessedSubdatasDirty = false;
		}

		std::vector<entt::meta_data> visibleSubdatas;
		for (const auto subDataId : m_PreprocessedSubdatas)
		{
			entt::meta_data subdata = structType.data(subDataId);
			
			if (!m_SubdataParser.ShouldHideData(subdata, structInstance))
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

#ifndef DOCTEST_CONFIG_DISABLE
	void StructWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
	{
		const auto structType = m_DataSpec.GetType();
		auto structInstance = m_DataSpec.GetValue();
		std::deque<entt::meta_data> subdatas;
		for (const auto subdata : structType.data())
		{
			subdatas.push_front(subdata);
		}
		dataStack.emplace_back(m_DataSpec.Data, m_DataSpec.bIsSeqElement, elementIndex);
		for (const auto subdata : subdatas)
		{
			DataSpec subdataSpec{ subdata, m_DataSpec.ComponentInstance, structInstance, true, false };
			ConstructBasicDataWidget(subdataSpec, subdata.type(), true)->Test(reg, entity, dataStack);
		}
		dataStack.pop_back();
	}
#endif

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

	void StructWidget::DrawSubdataWidget(entt::meta_data subdata, entt::meta_any& structInstance)
	{
		uint32_t aggregatedSubdataId = GetAggregatedDataID(subdata);
		if (m_SubdataWidgets.find(aggregatedSubdataId) != m_SubdataWidgets.cend())
		{
			if (m_SubdataWidgets[aggregatedSubdataId])
			{
				m_SubdataWidgets[aggregatedSubdataId]->Draw(m_DataSpec.ComponentInstance, structInstance);
			}
		}
		else
		{
			DataSpec subdataSpec{ subdata, m_DataSpec.ComponentInstance, structInstance, true, false };
			m_SubdataWidgets[aggregatedSubdataId] = ConstructBasicDataWidget(subdataSpec, subdata.type());
		}
	}

}
