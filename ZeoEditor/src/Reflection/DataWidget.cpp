#include "Reflection/DataWidget.h"

#include <misc/cpp/imgui_stdlib.h>
#include <IconsFontAwesome5.h>

#include "Engine/GameFramework/ComponentHelpers.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Core/EditorManager.h"
#include "Editors/EditorBase.h"
#include "Engine/Core/AssetRegistry.h"

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
				return CreateRef<ScalarNDataWidget<float>>(dataSpec, bIsTest, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f");
			case BasicMetaType::DOUBLE:
				return CreateRef<ScalarNDataWidget<double>>(dataSpec, bIsTest, ImGuiDataType_Double, -DBL_MAX, DBL_MAX, "%.4lf");
			case BasicMetaType::ENUM:
				return CreateRef<EnumDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::STRING:
				return CreateRef<StringDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::VEC2:
				return CreateRef<ScalarNDataWidget<glm::vec2, 2, float>>(dataSpec, bIsTest, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f");
			case BasicMetaType::VEC3:
				return CreateRef<ScalarNDataWidget<glm::vec3, 3, float>>(dataSpec, bIsTest, ImGuiDataType_Float, -FLT_MAX, FLT_MAX, "%.3f");
			case BasicMetaType::VEC4:
				return CreateRef<ColorDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::TEXTURE:
				return CreateRef<Texture2DDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::PARTICLE:
				return CreateRef<ParticleTemplateDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::MESH:
				return CreateRef<MeshDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::MATERIAL:
				return CreateRef<MaterialDataWidget>(dataSpec, bIsTest);
			case BasicMetaType::SHADER:
				return CreateRef<ShaderDataWidget>(dataSpec, bIsTest);
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

	void DataWidget::InvokeOnDataValueEditChangeCallback(entt::meta_data data, std::any oldValue, int32_t elementIndex)
	{
		if (m_bIsTest) return;

		ZE_TRACE("Value changed during edit!");
		IComponent* comp = m_DataSpec.ComponentInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		if (comp->ComponentHelper)
		{
			comp->ComponentHelper->OnComponentDataValueEditChange(data.id(), oldValue, elementIndex);
		}
	}

	void DataWidget::InvokePostDataValueEditChangeCallback(entt::meta_data data, std::any oldValue, int32_t elementIndex)
	{
		if (m_bIsTest) return;

		ZE_TRACE("Value changed after edit!");
		IComponent* comp = m_DataSpec.ComponentInstance.try_cast<IComponent>();
		ZE_CORE_ASSERT(comp);
		if (comp->ComponentHelper)
		{
			comp->ComponentHelper->PostComponentDataValueEditChange(data.id(), oldValue, elementIndex);
		}
	}

	BoolDataWidget::BoolDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void BoolDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;

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

	void EnumDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;

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
							InvokePostDataValueEditChangeCallback(m_DataSpec.Data, m_OldBuffer, m_DataSpec.ElementIndex);
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

	void StringDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;

		ImGui::InputText("##String", &m_Buffer, ImGuiInputTextFlags_AutoSelectAll);
		// For tabbing (we must force set value back in this case or the buffer will be reset on the next draw)
		if (Input::IsKeyPressed(Key::Tab) && ImGui::GetFocusID() == ImGui::GetItemID())
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

	void ColorDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;

		bool bChanged = ImGui::ColorEdit4("", glm::value_ptr(m_Buffer));
		// For dragging
		if (bChanged && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			SetValueToData(false);
		}
		// For tabbing (we must force set value back in this case or the buffer will be reset on the next draw)
		if (Input::IsKeyPressed(Key::Tab) && ImGui::GetFocusID() == ImGui::GetItemID())
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

	void Texture2DDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;

		// Make sure browser widget + dropdown button can reach desired size
		float rightPadding = m_DataSpec.bIsSeqElement ? GetDropdownWidth() : -1.0f;
		// Texture2D asset browser
		auto [bIsBufferChanged, retSpec] = m_Browser.Draw(m_Buffer ? m_Buffer->GetID() : std::string{}, rightPadding, [](){});
		if (bIsBufferChanged)
		{
			m_Buffer = retSpec ? Texture2DLibrary::Get().LoadAsset(retSpec->Path) : AssetHandle<Texture2D>{};
			SetValueToData();
		}

		PostDraw();
	}

#ifndef DOCTEST_CONFIG_DISABLE
	void Texture2DDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
{
		m_Buffer = Texture2DAssetLibrary::Get().LoadAsset("assets/textures/Ship.png.zasset");
		SetValueToData();
		CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
	}
#endif

	ParticleTemplateDataWidget::ParticleTemplateDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void ParticleTemplateDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;

		// Make sure browser widget + dropdown button can reach desired size
		float rightPadding = m_DataSpec.bIsSeqElement ? GetDropdownWidth() : -1.0f;
		// Particle template asset browser
		auto [bIsBufferChanged, retSpec] = m_Browser.Draw(m_Buffer ? m_Buffer->GetID() : std::string{}, rightPadding, [this]()
		{
			if (ImGui::MenuItem(ICON_FA_REDO "  Resimulate"))
			{
				m_DataSpec.ComponentInstance.cast<ParticleSystemComponent>().Instance->Resimulate();
			}
		});
		if (bIsBufferChanged)
		{
			m_Buffer = retSpec ? ParticleTemplateLibrary::Get().LoadAsset(retSpec->Path) : AssetHandle<ParticleTemplate>{};
			SetValueToData();
		}

		PostDraw();
	}

#ifndef DOCTEST_CONFIG_DISABLE
	void ParticleTemplateDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
{
		m_Buffer = ParticleTemplateLibrary::Get().LoadAsset("assets/particles/Test.zasset");
		SetValueToData();
		CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
	}
#endif

	MeshDataWidget::MeshDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void MeshDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;

		// Make sure browser widget + dropdown button can reach desired size
		float rightPadding = m_DataSpec.bIsSeqElement ? GetDropdownWidth() : -1.0f;
		// Mesh asset browser
		auto [bIsBufferChanged, retSpec] = m_Browser.Draw(m_Buffer ? m_Buffer->GetID() : std::string{}, rightPadding, []() {});
		if (bIsBufferChanged)
		{
			m_Buffer = retSpec ? MeshAssetLibrary::Get().LoadAsset(retSpec->Path) : AssetHandle<MeshAsset>{};
			SetValueToData();
		}

		PostDraw();
	}

#ifndef DOCTEST_CONFIG_DISABLE
	void MeshDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
	{
		m_Buffer = MeshAssetLibrary::GetDefaultSphereMesh();
		SetValueToData();
		CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
	}
#endif

	MaterialDataWidget::MaterialDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void MaterialDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;

		// Make sure browser widget + dropdown button can reach desired size
		float rightPadding = m_DataSpec.bIsSeqElement ? GetDropdownWidth() : -1.0f;
		// Material asset browser
		auto [bIsBufferChanged, retSpec] = m_Browser.Draw(m_Buffer ? m_Buffer->GetID() : std::string{}, rightPadding, []() {});
		if (bIsBufferChanged)
		{
			m_Buffer = retSpec ? MaterialLibrary::Get().LoadAsset(retSpec->Path) : AssetHandle<Material>{};
			SetValueToData();
		}

		PostDraw();
	}

#ifndef DOCTEST_CONFIG_DISABLE
	void MaterialDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
	{
		m_Buffer = MaterialAssetLibrary::GetDefaultMaterialAsset();
		SetValueToData();
		CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
	}
#endif

	ShaderDataWidget::ShaderDataWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
	}

	void ShaderDataWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;

		// Make sure browser widget + dropdown button can reach desired size
		float rightPadding = m_DataSpec.bIsSeqElement ? GetDropdownWidth() : -1.0f;
		// Shader asset browser
		auto [bIsBufferChanged, retSpec] = m_Browser.Draw(m_Buffer ? m_Buffer->GetID() : std::string{}, rightPadding, []() {});
		if (bIsBufferChanged)
		{
			m_Buffer = retSpec ? ShaderAssetLibrary::Get().LoadAsset(retSpec->Path) : AssetHandle<ShaderAsset>{};
			SetValueToData();
		}

		PostDraw();
	}

#ifndef DOCTEST_CONFIG_DISABLE
	void ShaderDataWidget::TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
	{
		m_Buffer = ShaderAssetLibrary::GetDefaultShaderAsset();
		SetValueToData();
		CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
	}
#endif

	bool ContainerWidget::PreDraw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		m_DataSpec.Update(compInstance, instance, elementIndex);

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
			m_bIsFixedSize = DoesPropExist(PropertyType::FixedSizeContainer, m_DataSpec.Data);
			DataSpec elementDataSpec{ dataSpec.Data, dataSpec.ComponentInstance, dataSpec.Instance, false, true };
			auto seqView = m_DataSpec.GetValue().as_sequence_container();
			m_ElementWidgetTemplate = ConstructBasicDataWidget(elementDataSpec, seqView.value_type());
		}
	}

	void SequenceContainerWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!m_ElementWidgetTemplate) return;

		if (!PreDraw(compInstance, instance, elementIndex)) return;

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
			// Drag to re-arrange sequence container elements
			if (!m_bIsFixedSize)
			{
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Drag to re-arrange elements");
				}

				uint32_t id = GetAggregatedDataID(m_DataSpec.Data);
				char dragTypeBuffer[DRAG_DROP_PAYLOAD_TYPE_SIZE];
				_itoa_s(id, dragTypeBuffer, 10);
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload(dragTypeBuffer, &i, sizeof(uint32_t));
					ImGui::Text("Place it here");

					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::MyAcceptDragDropPayload(dragTypeBuffer))
					{
						ZE_CORE_ASSERT(payload->DataSize == sizeof(uint32_t));

						auto sourceIndex = *(const uint32_t*)payload->Data;
						auto targetIndex = i;
						bool bMoveDownward = targetIndex >= sourceIndex;

						// Insert to target location
						{
							auto sourceElement = seqView[sourceIndex];
							auto targetIt = it;
							auto retIt = seqView.insert(bMoveDownward ? ++targetIt : targetIt, sourceElement);
							ZE_CORE_ASSERT(retIt);
						}

						// Erase from source location
						{
							auto sourceIt = seqView.begin();
							for (uint32_t j = 0; j < sourceIndex; ++j)
							{
								++sourceIt;
							}
							auto retIt = seqView.erase(bMoveDownward ? sourceIt : ++sourceIt);
							ZE_CORE_ASSERT(retIt);
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
							m_ElementWidgetTemplate->Draw(compInstance, elementInstance, i);
						}

						ImGui::TreePop();
					}
				}
				else
				{
					// Make sure element widget + dropdown button can reach desired size
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - GetDropdownWidth());
					// Draw element widget
					m_ElementWidgetTemplate->Draw(compInstance, elementInstance, i);
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

		if (m_bIsFixedSize)
		{
			ImGui::BeginDisabled();
		}
		{
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
					// TODO:
					InvokePostDataValueEditChangeCallback(m_DataSpec.Data, {}, -1);
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Remove all elements");
			}
		}
		if (m_bIsFixedSize)
		{
			ImGui::EndDisabled();
		}
	}

	void SequenceContainerWidget::DrawContainerElementOperationWidget(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator& it)
	{
		if (m_bIsFixedSize)
		{
			ImGui::BeginDisabled();
		}
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
		if (m_bIsFixedSize)
		{
			ImGui::EndDisabled();
		}
	}

	entt::meta_sequence_container::iterator SequenceContainerWidget::InsertValue(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it)
	{
		const auto elementType = seqView.value_type();
		auto retIt = seqView.insert(it, elementType.construct()); // Construct the pre-registered type with default value
		if (retIt)
		{
			// TODO:
			InvokePostDataValueEditChangeCallback(m_DataSpec.Data, {}, -1);
		}
		else
		{
			auto dataName = GetMetaObjectDisplayName(m_DataSpec.Data);
			ZE_CORE_ASSERT(false, "Failed to insert with data: '{0}'! Please check if its type is properly registered.", *dataName);
		}
		return retIt;
	}

	entt::meta_sequence_container::iterator SequenceContainerWidget::EraseValue(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it)
	{
		auto retIt = seqView.erase(it);
		if (retIt)
		{
			// TODO:
			InvokePostDataValueEditChangeCallback(m_DataSpec.Data, {}, -1);
		}
		else
		{
			auto dataName = GetMetaObjectDisplayName(m_DataSpec.Data);
			ZE_CORE_ERROR("Failed to erase with data: {0}!", *dataName);
		}
		return retIt;
	}

	AssociativeContainerWidget::AssociativeContainerWidget(DataSpec& dataSpec, bool bIsTest)
	{
		Init(dataSpec, bIsTest);
		if (!bIsTest)
		{
			m_bIsFixedSize = DoesPropExist(PropertyType::FixedSizeContainer, m_DataSpec.Data);
		}
	}

	void AssociativeContainerWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;



		PostDraw();
	}

	void AssociativeContainerWidget::DrawContainerOperationWidget()
	{
		if (m_bIsFixedSize)
		{
			ImGui::BeginDisabled();
		}
		{

		}
		if (m_bIsFixedSize)
		{
			ImGui::EndDisabled();
		}
	}

	void AssociativeContainerWidget::DrawContainerElementOperationWidget(entt::meta_associative_container& assView, entt::meta_associative_container::iterator& it)
	{
		if (m_bIsFixedSize)
		{
			ImGui::BeginDisabled();
		}
		{

		}
		if (m_bIsFixedSize)
		{
			ImGui::EndDisabled();
		}
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

	bool StructWidget::PreDraw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		m_DataSpec.Update(compInstance, instance, elementIndex);

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

	void StructWidget::Draw(entt::meta_any& compInstance, entt::meta_any& instance, int32_t elementIndex)
	{
		if (!PreDraw(compInstance, instance, elementIndex)) return;

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
