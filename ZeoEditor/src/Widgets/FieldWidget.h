#pragma once

#include <deque>
#include <entt.hpp>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Engine/Core/Core.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Scripting/ScriptFieldInstance.h"
#include "Inspectors/ComponentFieldInstance.h"
#include "Widgets/AssetBrowser.h"
#include "Widgets/EntityBrowser.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Utils/ReflectionUtils.h"

namespace ZeoEngine {

	class FieldInstanceBase;

	namespace Utils	{

		ImGuiDataType FieldTypeToImGuiDataType(FieldType type);
		const char* GetOutputFormatByFieldType(FieldType type);

		template<typename FieldInstance>
		Scope<IFieldWidget> ConstructFieldWidget(FieldType type, UUID widgetID, Ref<FieldInstance> fieldInstance)
		{
			if constexpr (std::is_same_v<FieldInstance, ComponentFieldInstance>)
			{
				auto customWidget = ReflectionUtils::GetPropertyValue<CustomWidgetConstructFunc>(Reflection::CustomWidget, fieldInstance->GetFieldData());
				if (customWidget)
				{
					return (*customWidget)(widgetID, std::move(fieldInstance));
				}
			}

			switch (type)
			{
				case FieldType::Bool:
					return CreateScope<BoolFieldWidget<FieldInstance>>(widgetID, std::move(fieldInstance));
				case FieldType::I8:
					return CreateScope<ScalarNFieldWidget<FieldInstance, I8>>(widgetID, std::move(fieldInstance));
				case FieldType::U8:
					return CreateScope<ScalarNFieldWidget<FieldInstance, U8>>(widgetID, std::move(fieldInstance));
				case FieldType::I16:
					return CreateScope<ScalarNFieldWidget<FieldInstance, I16>>(widgetID, std::move(fieldInstance));
				case FieldType::U16:
					return CreateScope<ScalarNFieldWidget<FieldInstance, U16>>(widgetID, std::move(fieldInstance));
				case FieldType::I32:
					return CreateScope<ScalarNFieldWidget<FieldInstance, I32>>(widgetID, std::move(fieldInstance));
				case FieldType::U32:
					return CreateScope<ScalarNFieldWidget<FieldInstance, U32>>(widgetID, std::move(fieldInstance));
				case FieldType::I64:
					return CreateScope<ScalarNFieldWidget<FieldInstance, I64>>(widgetID, std::move(fieldInstance));
				case FieldType::U64:
					return CreateScope<ScalarNFieldWidget<FieldInstance, U64>>(widgetID, std::move(fieldInstance));
				case FieldType::Float:
					return CreateScope<ScalarNFieldWidget<FieldInstance, float>>(widgetID, std::move(fieldInstance));
				case FieldType::Double:
					return CreateScope<ScalarNFieldWidget<FieldInstance, double>>(widgetID, std::move(fieldInstance));
				case FieldType::Vec2:
					return CreateScope<ScalarNFieldWidget<FieldInstance, Vec2, 2, float>>(widgetID, std::move(fieldInstance));
				case FieldType::Vec3:
					return CreateScope<ScalarNFieldWidget<FieldInstance, Vec3, 3, float>>(widgetID, std::move(fieldInstance));
				case FieldType::Vec4:
					return CreateScope<ColorFieldWidget<FieldInstance>>(widgetID, std::move(fieldInstance));
				case FieldType::Enum:
					return CreateScope<EnumFieldWidget<FieldInstance>>(widgetID, std::move(fieldInstance));
				case FieldType::String:
					return CreateScope<StringFieldWidget<FieldInstance>>(widgetID, std::move(fieldInstance));
				case FieldType::SeqCon:
					if constexpr (!std::is_same_v<FieldInstance, ScriptFieldInstance>)
					{
						return CreateScope<SequenceContainerFieldWidget<FieldInstance>>(widgetID, std::move(fieldInstance));
					}
					break;
				case FieldType::Asset:
					return CreateScope<AssetFieldWidget<FieldInstance>>(widgetID, std::move(fieldInstance));
				case FieldType::Entity:
					return CreateScope<EntityFieldWidget<FieldInstance>>(widgetID, std::move(fieldInstance));
			}

			ZE_CORE_ASSERT(false);
			return nullptr;
		}

		bool IsFieldTypeBufferBased(FieldType type);

		template<typename FieldInstance>
		constexpr bool IsFieldSequenceContainerElement()
		{
			return std::is_same_v<FieldInstance, ComponentSequenceContainerElementFieldInstance> || std::is_same_v<FieldInstance, ScriptSequenceContainerFieldInstance>;
		}

		float GetContainerDropdownWidth();
		
	}

	class IFieldWidget
	{
	public:
		virtual ~IFieldWidget() = default;

		virtual void Draw() = 0;
		virtual void DrawElement(U32 index) = 0;
	};

	template<typename FieldInstance>
	class FieldWidgetBase : public IFieldWidget
	{
	public:
		FieldWidgetBase(UUID widgetID, Ref<FieldInstance> fieldInstance)
			: m_WidgetID(widgetID)
			, m_Instance(std::move(fieldInstance))
		{
		}

		UUID GetWidgetID() const { return m_WidgetID; }
		Ref<FieldInstance> GetFieldInstance() const { return m_Instance; }

		U32 GetFieldSize() const
		{
			if constexpr (std::is_same_v<FieldInstance, ScriptSequenceContainerFieldInstance>)
			{
				return m_Instance->GetElementSize();
			}
			else
			{
				return m_Instance->GetFieldSize();
			}
		}

		FieldType GetFieldType() const
		{
			if constexpr (std::is_same_v<FieldInstance, ScriptSequenceContainerFieldInstance>)
			{
				return m_Instance->GetElementType();
			}
			else
			{
				return m_Instance->GetFieldType();
			}
		}

		virtual void Draw() override
		{
			ImGui::AlignTextToFramePadding();
			// Field name
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_SpanAvailWidth;
			ImGui::TreeNodeEx(m_Instance->GetFieldName(), flags);
			// Field tooltip
			if (ImGui::IsItemHovered())
			{
				if (const std::string tooltip = m_Instance->GetFieldTooltip(); !tooltip.empty())
				{
					ImGui::SetTooltipWithPadding(tooltip.c_str());
				}
			}
			// Switch to the right column
			ImGui::TableNextColumn();
			// Align widget width to the right side
			ImGui::SetNextItemWidth(-1.0f);

			this->BeginDisabledDraw([this]()
			{
				ProcessDraw();
			});

			// Switch to the next row
			ImGui::TableNextColumn();
		}

		virtual void ProcessDraw() = 0;

	protected:
		virtual void DrawElement(U32 index) override final
		{
			if constexpr (Utils::IsFieldSequenceContainerElement<FieldInstance>())
			{
				GetFieldInstance()->SetIndex(index);
				ProcessDraw();
			}
		}

		void ApplyValueToInstance(const void* value)
		{
			if (!IsValueChanged(value)) return;
			
			SetValue(value);
			m_Instance->OnFieldValueChanged();
		}

		template<typename Func>
		void BeginDisabledDraw(Func func)
		{
			const bool bDisabled = m_Instance->IsFieldDisabled();
			if (bDisabled)
			{
				ImGui::BeginDisabled();
			}
			func();
			if (bDisabled)
			{
				ImGui::EndDisabled();
			}
		}

	private:
		virtual bool IsValueChanged(const void* value)
		{
			return memcmp(value, m_Instance->GetValueRaw(), GetFieldSize()) != 0;
		}

		virtual void SetValue(const void* value) {}

	private:
		UUID m_WidgetID = 0;
		Ref<FieldInstance> m_Instance;
	};

	template<typename FieldInstance>
	class FieldWidgetBufferBase : public FieldWidgetBase<FieldInstance>
	{
	public:
		using FieldWidgetBase<FieldInstance>::FieldWidgetBase;

	protected:
		void* GetBufferData() const { return m_Buffer.Data; }
		void SetBufferData(U8* data) { m_Buffer.Data = data; }

		void AllocateBuffer()
		{
			m_Buffer = Buffer(this->GetFieldSize());
		}

		void FreeBuffer()
		{
			m_Buffer.Release();
		}

		void UpdateBufferFromInstance()
		{
			const void* value = this->GetFieldInstance()->GetValueRaw();
			memcpy(m_Buffer.Data, value, this->GetFieldSize());
		}

		bool IsEditActive() const { return m_bIsEditActive; }
		void SetEditActive(bool bActive) { m_bIsEditActive = bActive; }

	private:
		virtual void SetValue(const void* value) override
		{
			this->GetFieldInstance()->SetValueRaw(value);
		}

	private:
		Buffer m_Buffer;
		bool m_bIsEditActive = false;
	};

	template<typename FieldInstance>
	class BoolFieldWidget : public FieldWidgetBase<FieldInstance>
	{
	public:
		using FieldWidgetBase<FieldInstance>::FieldWidgetBase;

	private:
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			bool value = fieldInstance->template GetValue<bool>();
			if (ImGui::Checkbox("", &value))
			{
				this->ApplyValueToInstance(&value);
			}
		}

		virtual void SetValue(const void* value) override
		{
			bool bValue = *static_cast<const bool*>(value);
			this->GetFieldInstance()->SetValue(bValue);
		}
	};

	template<typename FieldInstance, typename Type, U32 N = 1, typename BaseType = Type>
	class ScalarNFieldWidget : public FieldWidgetBufferBase<FieldInstance>
	{
	public:
		ScalarNFieldWidget(UUID widgetID, Ref<FieldInstance> fieldInstance)
			: FieldWidgetBufferBase<FieldInstance>(widgetID, std::move(fieldInstance))
		{
			static_assert(N == 1 || N == 2 || N == 3, "N can only be 1, 2 or 3!");

			this->AllocateBuffer();
		}
		~ScalarNFieldWidget()
		{
			this->FreeBuffer();
		}

	private:
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			const float dragSpeed = fieldInstance->GetDragSpeed();
			const auto min = fieldInstance->template GetDragMin<BaseType>();
			const auto max = fieldInstance->template GetDragMax<BaseType>();
			const ImGuiSliderFlags clampMode = fieldInstance->IsClampOnlyDuringDragging() ? 0 : ImGuiSliderFlags_AlwaysClamp;

			void* buffer = this->IsEditActive() ? this->GetBufferData() : fieldInstance->GetValueRaw();
			const FieldType fieldType = this->GetFieldType();
			bool bChanged = ImGui::DragScalarNEx("", Utils::FieldTypeToImGuiDataType(fieldType), buffer, N, dragSpeed, &min, &max, Utils::GetOutputFormatByFieldType(fieldType), clampMode);
			if (bChanged && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				this->ApplyValueToInstance(this->GetBufferData());
			}
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				this->SetEditActive(false);
				this->ApplyValueToInstance(this->GetBufferData());
			}
			if (ImGui::IsItemActivated())
			{
				this->SetEditActive(true);
				this->UpdateBufferFromInstance();
			}
		}
	};

	template<typename FieldInstance>
	class ColorFieldWidget : public FieldWidgetBufferBase<FieldInstance>
	{
	public:
		ColorFieldWidget(UUID widgetID, Ref<FieldInstance> fieldInstance)
			: FieldWidgetBufferBase<FieldInstance>(widgetID, std::move(fieldInstance))
		{
			this->AllocateBuffer();
		}
		~ColorFieldWidget()
		{
			this->FreeBuffer();
		}

	private:
		virtual void ProcessDraw() override
		{
			void* buffer = this->IsEditActive() ? this->GetBufferData() : this->GetFieldInstance()->GetValueRaw();
			bool bChanged = ImGui::ColorEdit4("", static_cast<float*>(buffer));
			if (bChanged && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				this->ApplyValueToInstance(this->GetBufferData());
			}
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				this->SetEditActive(false);
				this->ApplyValueToInstance(this->GetBufferData());
			}
			if (ImGui::IsItemActivated())
			{
				this->SetEditActive(true);
				this->UpdateBufferFromInstance();
			}
		}
	};

	template<typename FieldInstance>
	class EnumFieldWidget : public FieldWidgetBase<FieldInstance>
	{
	public:
		EnumFieldWidget(UUID widgetID, Ref<FieldInstance> fieldInstance)
			: FieldWidgetBase<FieldInstance>(widgetID, std::move(fieldInstance))
		{
			InitEnumEntries();
		}

	private:
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			auto value = fieldInstance->GetValueInternal();
			const char* currentName = ReflectionUtils::GetEnumDisplayName(value);
			if (ImGui::BeginCombo("", currentName))
			{
				for (const auto enumEntry : m_EnumEntries)
				{
					const char* enumEntryName = ReflectionUtils::GetMetaObjectName(enumEntry);
					bool bIsSelected = ImGui::Selectable(enumEntryName);
					if (ImGui::IsItemHovered())
					{
						if (auto tooltip = ReflectionUtils::GetPropertyValue<const char*>(Reflection::Tooltip, enumEntry))
						{
							ImGui::SetTooltipWithPadding(*tooltip);
						}
					}
					if (bIsSelected)
					{
						this->ApplyValueToInstance(enumEntry.get({}).data());
					}
				}

				ImGui::EndCombo();
			}
		}

		virtual void SetValue(const void* value) override
		{
			this->GetFieldInstance()->SetValueRaw(value);
		}

		void InitEnumEntries()
		{
			m_EnumEntries.clear();
			const auto type = this->GetFieldInstance()->GetFieldValueType();
			for (const auto enumData : type.data())
			{
				// Reverse order
				m_EnumEntries.push_front(enumData);
			}
		}

	private:
		std::deque<entt::meta_data> m_EnumEntries;
	};

	template<>
	class EnumFieldWidget<ScriptFieldInstance> : public FieldWidgetBase<ScriptFieldInstance>
	{
	public:
		using FieldWidgetBase::FieldWidgetBase;

	private:
		virtual void ProcessDraw() override
		{
			ZE_CORE_ASSERT(false);
		}

		virtual void SetValue(const void* value) override
		{
			ZE_CORE_ASSERT(false);
		}
	};

	template<>
	class EnumFieldWidget<ScriptSequenceContainerFieldInstance> : public EnumFieldWidget<ScriptFieldInstance>
	{
	public:
		using EnumFieldWidget<ScriptFieldInstance>::EnumFieldWidget;

	private:
		virtual void ProcessDraw() override
		{
			ZE_CORE_ASSERT(false);
		}

		virtual void SetValue(const void* value) override
		{
			ZE_CORE_ASSERT(false);
		}
	};

	/** Map from widget ID to cached string. */
	extern std::unordered_map<UUID, std::string> s_FieldStringBuffer;

	template<typename FieldInstance>
	class StringFieldWidget : public FieldWidgetBufferBase<FieldInstance>
	{
	public:
		using FieldWidgetBufferBase<FieldInstance>::FieldWidgetBufferBase;

		~StringFieldWidget()
		{
			const auto it = s_FieldStringBuffer.find(this->GetWidgetID());
			if (it != s_FieldStringBuffer.end())
			{
				s_FieldStringBuffer.erase(it);
			}
		}

	private:
		virtual void ProcessDraw() override
		{
			if (SceneUtils::IsLevelRuntime())
			{
				if (this->IsEditActive())
				{
					auto* value = static_cast<std::string*>(this->GetBufferData());
					ImGui::InputText("", value, ImGuiInputTextFlags_AutoSelectAll);
				}
				else
				{
					// Store the value first to prevent being destroyed
					std::string value = this->GetFieldInstance()->template GetValue<std::string>();
					ImGui::InputText("", &value, ImGuiInputTextFlags_AutoSelectAll);
				}
			}
			else
			{
				std::string* buffer = this->IsEditActive() ? static_cast<std::string*>(this->GetBufferData()) : static_cast<std::string*>(this->GetFieldInstance()->GetValueRaw())/* Use string buffer pointer directly to prevent an unnecessary copy */;
				ImGui::InputText("", buffer, ImGuiInputTextFlags_AutoSelectAll);
			}
			if (ImGui::IsItemDeactivated())
			{
				this->SetEditActive(false);
			}
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				this->ApplyValueToInstance(this->GetBufferData());
			}
			if (ImGui::IsItemActivated())
			{
				this->SetEditActive(true);
				UpdateStringBufferFromInstance();
			}
		}

		void UpdateStringBufferFromInstance()
		{
			auto& stringBuffer = s_FieldStringBuffer[this->GetWidgetID()];
			auto value = this->GetFieldInstance()->template GetValue<std::string>(); // Copy string
			stringBuffer = std::move(value);
			this->SetBufferData(reinterpret_cast<U8*>(&stringBuffer));
		}

		virtual bool IsValueChanged(const void* value) override
		{
			const auto& valueStr = *static_cast<const std::string*>(value);
			const auto oldValueStr = this->GetFieldInstance()->template GetValue<std::string>();
			return valueStr != oldValueStr;
		}

		virtual void SetValue(const void* value) override
		{
			this->GetFieldInstance()->SetValue(*static_cast<const std::string*>(value));
		}
	};

	template<typename FieldInstance>
	class AssetFieldWidget : public FieldWidgetBase<FieldInstance>
	{
	public:
		AssetFieldWidget(UUID widgetID, Ref<FieldInstance> fieldInstance)
			: FieldWidgetBase<FieldInstance>(widgetID, std::move(fieldInstance))
		{
			m_Browser.TypeID = this->GetFieldInstance()->GetAssetTypeID();
			if (!m_Browser.TypeID)
			{
				ZE_CORE_WARN("Asset field '{0}' does not register an 'AssetType' property!", this->GetFieldInstance()->GetFieldName());
			}
		}

	private:
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			auto value = fieldInstance->template GetValue<AssetHandle>();
			float rightPadding = 0.0f;
			if constexpr (Utils::IsFieldSequenceContainerElement<FieldInstance>())
			{
				rightPadding = Utils::GetContainerDropdownWidth();
			}
			bool bAllowClear = m_Browser.TypeID != Shader::TypeID();
			if (m_Browser.Draw(value, rightPadding, []() {}, bAllowClear))
			{
				this->ApplyValueToInstance(&value);
			}
		}

		virtual void SetValue(const void* value) override
		{
			AssetHandle handle = *static_cast<const AssetHandle*>(value);
			this->GetFieldInstance()->SetValue(handle);
		}

	private:
		AssetBrowser m_Browser;
	};

	template<typename FieldInstance>
	class EntityFieldWidget : public FieldWidgetBase<FieldInstance>
	{
	public:
		using FieldWidgetBase<FieldInstance>::FieldWidgetBase;

	private:
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			auto value = fieldInstance->template GetValue<UUID>();
			float rightPadding = 0.0f;
			if constexpr (Utils::IsFieldSequenceContainerElement<FieldInstance>())
			{
				rightPadding = Utils::GetContainerDropdownWidth();
			}
			if (m_Browser.Draw(value, rightPadding))
			{
				this->ApplyValueToInstance(&value);
			}
		}

		virtual void SetValue(const void* value) override
		{
			auto handle = *static_cast<const UUID*>(value);
			this->GetFieldInstance()->SetValue(handle);
		}

	private:
		EntityBrowser m_Browser;
	};

	template<typename FieldInstance>
	class ContainerFieldWidget : public FieldWidgetBase<FieldInstance>
	{
	public:
		using FieldWidgetBase<FieldInstance>::FieldWidgetBase;

		virtual void Draw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			const auto size = GetContainerSize();
			// Field name
			ImGuiTreeNodeFlags flags = size > 0 ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
			bool bIsTreeExpanded = ImGui::TreeNodeEx(fieldInstance->GetFieldName(), flags);
			// Field tooltip
			if (ImGui::IsItemHovered())
			{
				if (const std::string tooltip = fieldInstance->GetFieldTooltip(); !tooltip.empty())
				{
					ImGui::SetTooltipWithPadding(tooltip.c_str());
				}
			}
			// Switch to the right column
			ImGui::TableNextColumn();
			// Add and clear buttons
			this->BeginDisabledDraw([this]()
			{
				DrawContainerOperationWidget();
			});
			// Switch to the next row
			ImGui::TableNextColumn();
			if (bIsTreeExpanded)
			{
				this->BeginDisabledDraw([this]()
				{
					this->ProcessDraw();
				});

				ImGui::TreePop();
			}
		}

	private:
		virtual SizeT GetContainerSize() = 0;
		virtual void DrawContainerOperationWidget() = 0;
	};

	// TODO: Merge this with SequenceContainerFieldWidget<ScriptSequenceContainerFieldInstance>
	template<typename FieldInstance>
	class SequenceContainerFieldWidget : public ContainerFieldWidget<FieldInstance>
	{
	public:
		SequenceContainerFieldWidget(UUID widgetID, Ref<FieldInstance> fieldInstance)
			: ContainerFieldWidget<FieldInstance>(widgetID, std::move(fieldInstance))
		{
			Init();
		}

	private:
		// TODO: Optimize string
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			auto seqView = GetSequenceView();
			const FieldType type = ReflectionUtils::MetaTypeToFieldType(seqView.value_type());
			const bool bIsElementBufferBased = Utils::IsFieldTypeBufferBased(type);
			// No need to detect size change for non-buffer-based type
			// as it uses template to draw each element
			if (bIsElementBufferBased)
			{
				const auto seqSize = GetContainerSize();
				const auto widgetSize = m_ElementFieldWidgets.size();
				if (seqSize != widgetSize)
				{
					// Add or remove element widgets based on size change
					OnSequenceContainerSizeChanged(type, seqSize, widgetSize);
				}
			}
			
			U32 i = 0;
			for (auto it = seqView.begin(); it != seqView.end();)
			{
				ImGui::AlignTextToFramePadding();

				std::string elementName;
				if (const auto func = ReflectionUtils::GetPropertyValue<CustomSequenceContainerElementNameFunc>(Reflection::CustomElementName, fieldInstance->GetFieldData()))
				{
					elementName = (*func)(fieldInstance->GetComponent(), fieldInstance->GetFieldID(), i);
				}
				else
				{
					elementName = std::to_string(i);
				}

				const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_SpanAvailWidth;
				// Element name
				ImGui::TreeNodeEx(elementName.c_str(), flags);
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("[%d]", i);
				}
				// Drag to re-arrange sequence container elements
				if (!m_bFixedSize)
				{
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltipWithPadding("[%d] Drag to re-arrange elements", i);
					}

					std::string widgetIDStr = std::to_string(this->GetWidgetID());
					if (ImGui::BeginDragDropSource())
					{
						ImGui::SetDragDropPayload(widgetIDStr.c_str(), &i, sizeof(U32));
						ImGui::Text("Place it here");

						ImGui::EndDragDropSource();
					}

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::MyAcceptDragDropPayload(widgetIDStr.c_str()))
						{
							ZE_CORE_ASSERT(payload->DataSize == sizeof(U32));

							const auto from = *static_cast<U32*>(payload->Data);
							// Backup element
							auto element = seqView[from];
							auto copyElement = element;

							// Remove element
							auto fromIt = seqView.begin();
							fromIt.operator++(from - 1);
							auto retIt = seqView.erase(fromIt);
							ZE_CORE_ASSERT(retIt);

							// Insert that element to the new location
							auto toIt = it;
							retIt = seqView.insert(toIt, copyElement);
							ZE_CORE_ASSERT(retIt);
						}

						ImGui::EndDragDropTarget();
					}
				}

				// Switch to the right column
				ImGui::TableNextColumn();
				// Push element index as ID
				ImGui::PushID(i);
				{
					// Make sure element widget + dropdown button can reach desired size
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - Utils::GetContainerDropdownWidth());
					// Draw element widgets
					if (bIsElementBufferBased)
					{
						m_ElementFieldWidgets[i]->DrawElement(i);
					}
					else
					{
						m_ElementFieldWidgets[0]->DrawElement(i);
					}

					ImGui::SameLine();

					// Insert and erase buttons
					DrawContainerElementOperationWidget(seqView, it, i);
					// Switch to the next row
					ImGui::TableNextColumn();
				}
				ImGui::PopID();

				if (it != seqView.end())
				{
					++it, ++i;
				}
			}
		}

		virtual SizeT GetContainerSize() override
		{
			return GetSequenceView().size();
		}

		virtual void DrawContainerOperationWidget() override
		{
			auto seqView = GetSequenceView();
			const auto size = seqView.size();
			ImGui::Text("%d elements", size);

			ImGui::SameLine();

			if (m_bFixedSize)
			{
				ImGui::BeginDisabled();
			}
			{
				if (ImGui::TransparentSmallButton(ICON_FA_PLUS))
				{
					InsertElement(seqView, seqView.end(), static_cast<U32>(size));
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Add an element to the last");
				}

				ImGui::SameLine();

				if (ImGui::TransparentSmallButton(ICON_FA_TRASH))
				{
					ClearElements(seqView);
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Remove all elements");
				}
			}
			if (m_bFixedSize)
			{
				ImGui::EndDisabled();
			}
		}

		auto GetSequenceView()
		{
			return this->GetFieldInstance()->GetValueInternal().as_sequence_container();
		}

		void Init()
		{
			const auto fieldInstance = this->GetFieldInstance();
			m_bFixedSize = ReflectionUtils::DoesPropertyExist(Reflection::FixedSizeContainer, fieldInstance->GetFieldData());
			const auto seqView = GetSequenceView();
			const auto size = seqView.size();
			const FieldType type = ReflectionUtils::MetaTypeToFieldType(seqView.value_type());
			if (Utils::IsFieldTypeBufferBased(type))
			{
				m_ElementFieldWidgets.reserve(size);
				for (SizeT i = 0; i < size; ++i)
				{
					// If element widget is buffer based, we have to create for each one
					// otherwise buffer is shared across elements, if one of them is being edited,
					// all others will display the same content
					m_ElementFieldWidgets.emplace_back(NewElementWidget(type));
				}
				
			}
			else
			{
				// If element widget is not buffer based, we do not need to create for each one,
				// just having a widget template and updating element index is enough
				// NOTE: In this case widget ID is also shared across elements!
				m_ElementFieldWidgets.emplace_back(NewElementWidget(type));
			}
		}

		void OnSequenceContainerSizeChanged(FieldType type, SizeT seqSize, SizeT widgetSize)
		{
			if (seqSize > widgetSize)
			{
				for (SizeT i = 0; i < seqSize - widgetSize; ++i)
				{
					m_ElementFieldWidgets.emplace_back(NewElementWidget(type));
				}
			}
			else
			{
				m_ElementFieldWidgets.erase(m_ElementFieldWidgets.begin() + seqSize, m_ElementFieldWidgets.end());
			}
		}

		void DrawContainerElementOperationWidget(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator& outIt, U32 index)
		{
			if (m_bFixedSize)
			{
				ImGui::BeginDisabled();
			}
			if (ImGui::BeginCombo("##SequenceContainerElementOperation", nullptr, ImGuiComboFlags_NoPreview))
			{
				if (ImGui::Selectable("Insert"))
				{
					outIt = InsertElement(seqView, outIt, index);
				}
				if (ImGui::Selectable("Erase"))
				{
					outIt = EraseElement(seqView, outIt, index);
				}

				ImGui::EndCombo();
			}
			if (m_bFixedSize)
			{
				ImGui::EndDisabled();
			}
		}

		entt::meta_sequence_container::iterator InsertElement(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it, U32 index)
		{
			const auto elementType = seqView.value_type();
			auto retIt = seqView.insert(it, elementType.construct()); // Construct the pre-registered type with default value
			const auto fieldInstance = this->GetFieldInstance();
			if (retIt)
			{
				const FieldType type = ReflectionUtils::MetaTypeToFieldType(seqView.value_type());
				m_ElementFieldWidgets.insert(m_ElementFieldWidgets.begin() + index, NewElementWidget(type));
				fieldInstance->OnFieldValueChanged();
			}
			else
			{
				const char* fieldName = fieldInstance->GetFieldName();
				ZE_CORE_ASSERT(false, "Failed to insert an element in '{0}'! Please check if its type is properly registered.", fieldName);
			}
			return retIt;
		}

		entt::meta_sequence_container::iterator EraseElement(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it, U32 index)
		{
			auto retIt = seqView.erase(it);
			ZE_CORE_ASSERT(retIt);
			m_ElementFieldWidgets.erase(m_ElementFieldWidgets.begin() + index);
			this->GetFieldInstance()->OnFieldValueChanged();
			return retIt;
		}

		void ClearElements(entt::meta_sequence_container& seqView)
		{
			if (seqView.clear())
			{
				m_ElementFieldWidgets.clear();
				this->GetFieldInstance()->OnFieldValueChanged();
			}
		}

		Scope<IFieldWidget> NewElementWidget(FieldType type)
		{
			const auto fieldInstance = this->GetFieldInstance();
			auto elementFieldInstance = CreateRef<ComponentSequenceContainerElementFieldInstance>(type, fieldInstance->GetFieldData(), fieldInstance->GetEntity(), fieldInstance->GetComponentID());
			return Utils::ConstructFieldWidget(type, UUID(), std::move(elementFieldInstance));
		}

	private:
		std::vector<Scope<IFieldWidget>> m_ElementFieldWidgets;
		bool m_bFixedSize = false;
	};

	template<>
	class SequenceContainerFieldWidget<ScriptSequenceContainerFieldInstance> : public ContainerFieldWidget<ScriptSequenceContainerFieldInstance>
	{
	public:
		SequenceContainerFieldWidget(UUID widgetID, Ref<ScriptSequenceContainerFieldInstance> fieldInstance)
			: ContainerFieldWidget(widgetID, std::move(fieldInstance))
		{
			Init();
		}

	private:
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = GetFieldInstance();
			if (SceneUtils::IsLevelRuntime())
			{
				// Update array data every time before drawing each element widgets
				fieldInstance->GetRuntimeArrayData();
			}

			const FieldType elementType = fieldInstance->GetElementType();
			const bool bIsElementBufferBased = Utils::IsFieldTypeBufferBased(elementType);
			// No need to detect size change for non-buffer-based type
			// as it uses template to draw each element
			if (bIsElementBufferBased)
			{
				const auto seqSize = GetContainerSize();
				const auto widgetSize = m_ElementFieldWidgets.size();
				if (seqSize != widgetSize)
				{
					// Add or remove element widgets based on size change
					OnSequenceContainerSizeChanged(seqSize, widgetSize);
				}
			}

			for (U32 i = 0; i < GetContainerSize(); ++i)
			{
				ImGui::AlignTextToFramePadding();

				const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_SpanAvailWidth;
				// Element name
				std::string elementName = std::to_string(i);
				ImGui::TreeNodeEx(elementName.c_str(), flags);
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("[%d]", i);
				}
				// Drag to re-arrange sequence container elements
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("[%d] Drag to re-arrange elements", i);
				}

				std::string widgetIDStr = std::to_string(GetWidgetID());
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload(widgetIDStr.c_str(), &i, sizeof(U32));
					ImGui::Text("Place it here");

					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::MyAcceptDragDropPayload(widgetIDStr.c_str()))
					{
						ZE_CORE_ASSERT(payload->DataSize == sizeof(U32));

						const auto sourceIndex = *static_cast<U32*>(payload->Data);
						const auto targetIndex = i;
						fieldInstance->MoveElement(sourceIndex, targetIndex);
					}

					ImGui::EndDragDropTarget();
				}

				// Switch to the right column
				ImGui::TableNextColumn();
				// Push element index as ID
				ImGui::PushID(i);
				{
					// Make sure element widget + dropdown button can reach desired size
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - Utils::GetContainerDropdownWidth());
					// Draw element widgets
					if (bIsElementBufferBased)
					{
						m_ElementFieldWidgets[i]->DrawElement(i);
					}
					else
					{
						m_ElementFieldWidgets[0]->DrawElement(i);
					}

					ImGui::SameLine();

					// Insert and erase buttons
					DrawContainerElementOperationWidget(i);
					// Switch to the next row
					ImGui::TableNextColumn();
				}
				ImGui::PopID();
			}
		}

		virtual SizeT GetContainerSize() override
		{
			return GetFieldInstance()->GetContainerSize();
		}

		virtual void DrawContainerOperationWidget() override
		{
			const auto size = GetContainerSize();
			ImGui::Text("%d elements", size);

			ImGui::SameLine();

			{
				if (ImGui::TransparentSmallButton(ICON_FA_PLUS))
				{
					InsertElement(static_cast<U32>(size), false);
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Add an element to the last");
				}

				ImGui::SameLine();

				if (ImGui::TransparentSmallButton(ICON_FA_TRASH))
				{
					ClearElements();
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltipWithPadding("Remove all elements");
				}
			}
		}

		void Init()
		{
			const FieldType elementType = GetFieldInstance()->GetElementType();
			if (Utils::IsFieldTypeBufferBased(elementType))
			{
				const auto size = GetContainerSize();
				m_ElementFieldWidgets.reserve(size);
				for (SizeT i = 0; i < size; ++i)
				{
					// If element widget is buffer based, we have to create for each one
					// otherwise buffer is shared across elements, if one of them is being edited,
					// all others will display the same content
					m_ElementFieldWidgets.emplace_back(NewElementWidget());
				}

			}
			else
			{
				// If element widget is not buffer based, we do not need to create for each one,
				// just having a widget template and updating element index is enough
				// NOTE: In this case widget ID is also shared across elements!
				m_ElementFieldWidgets.emplace_back(NewElementWidget());
			}
		}

		void OnSequenceContainerSizeChanged(SizeT seqSize, SizeT widgetSize)
		{
			if (seqSize > widgetSize)
			{
				for (U32 i = 0; i < seqSize - widgetSize; ++i)
				{
					m_ElementFieldWidgets.emplace_back(NewElementWidget());
				}
			}
			else
			{
				m_ElementFieldWidgets.erase(m_ElementFieldWidgets.begin() + seqSize, m_ElementFieldWidgets.end());
			}
		}

		void DrawContainerElementOperationWidget(U32 index)
		{
			if (ImGui::BeginCombo("##SequenceContainerElementOperation", nullptr, ImGuiComboFlags_NoPreview))
			{
				if (ImGui::Selectable("Insert"))
				{
					InsertElement(index, true);
				}
				if (ImGui::Selectable("Erase"))
				{
					EraseElement(index);
				}

				ImGui::EndCombo();
			}
		}

		void InsertElement(U32 index, bool bNeedsUpdateArray)
		{
			const auto fieldInstance = GetFieldInstance();
			if (fieldInstance->InsertDefault(index, bNeedsUpdateArray))
			{
				m_ElementFieldWidgets.insert(m_ElementFieldWidgets.begin() + index, NewElementWidget());
				fieldInstance->OnFieldValueChanged();
			}
			else
			{
				const char* fieldName = fieldInstance->GetFieldName();
				ZE_CORE_ASSERT(false, "Failed to insert an element in '{0}'!", fieldName);
			}
		}

		void EraseElement(U32 index)
		{
			const auto fieldInstance = GetFieldInstance();
			if (fieldInstance->Erase(index))
			{
				m_ElementFieldWidgets.erase(m_ElementFieldWidgets.begin() + index);
				fieldInstance->OnFieldValueChanged();
			}
			else
			{
				const char* fieldName = fieldInstance->GetFieldName();
				ZE_CORE_ERROR("Failed to erase an element in '{0}'!", fieldName);
			}
		}

		void ClearElements()
		{
			const auto fieldInstance = GetFieldInstance();
			fieldInstance->Clear();
			m_ElementFieldWidgets.clear();
			fieldInstance->OnFieldValueChanged();
		}

		Scope<IFieldWidget> NewElementWidget() const
		{
			const auto fieldInstance = GetFieldInstance();
			return Utils::ConstructFieldWidget(fieldInstance->GetElementType(), UUID(), fieldInstance);
		}

	private:
		std::vector<Scope<IFieldWidget>> m_ElementFieldWidgets;
	};
	
}
