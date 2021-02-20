#pragma once

#include <string>

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Core/ReflectionHelper.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

	template<typename T>
	void ShowPropertyTooltip(T metaObj)
	{
		if (ImGui::IsItemHovered())
		{
			auto tooltip = GetPropValue<const char*>(PropertyType::Tooltip, metaObj);
			if (tooltip)
			{
				ImGui::SetTooltip(*tooltip);
			}
		}
	}

	Ref<class DataWidget> ConstructBasicDataWidget(const DataSpec& dataSpec);

	static const ImGuiTreeNodeFlags DefaultDataTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
	static const ImGuiTreeNodeFlags EmptyContainerDataTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
	static const ImGuiTreeNodeFlags DefaultContainerDataTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;

	class DataWidget
	{
	public:
		// NOTE: Component instance should be updated as it will may get invalidated when a new entity's same type of component gets constructed
		virtual void Draw(const entt::meta_any& compInstance, int32_t elementIndex = -1) = 0;

	private:
		// Call this at the beginning of drawing!
		virtual bool PreDraw(const entt::meta_any& compInstance, int32_t elementIndex = -1) = 0;
		// Call this in the end of drawing!
		virtual void PostDraw() = 0;

	protected:
		void Init(const DataSpec& dataSpec)
		{
			m_DataSpec = dataSpec;
			if (!m_DataSpec.bIsSeqElement)
			{
				Draw(m_DataSpec.ComponentInstance);
			}
		}

	protected:
		DataSpec m_DataSpec;
	};

	template<typename T>
	class BasicDataWidgetT : public DataWidget
	{
	public:
		virtual bool PreDraw(const entt::meta_any& compInstance, int32_t elementIndex = -1) override
		{
			m_DataSpec.Update(compInstance, elementIndex);
			UpdateBuffer();

			if (elementIndex != -1) return true;

			// Data name
			bool bIsDataTreeExpanded = ImGui::TreeNodeEx(m_DataSpec.DataName, DefaultDataTreeNodeFlags);
			// Data tooltip
			ShowPropertyTooltip(m_DataSpec.Data);
			// Switch to the right column
			ImGui::TableNextColumn();
			// Align widget width to the right side
			ImGui::SetNextItemWidth(-1.0f);

			return bIsDataTreeExpanded;
		}

		virtual void PostDraw() override
		{
			if (!m_DataSpec.bIsSeqElement)
			{
				// Switch to the next row
				ImGui::TableNextColumn();
			}
		}

	protected:
		virtual void UpdateBuffer()
		{
			m_Buffer = GetValueFromData();
		}

		// TODO: Will copy?
		virtual T GetValueFromData()
		{
			return m_DataSpec.GetValue<T>();
		}

		void SetValueToData()
		{
			m_DataSpec.SetValue(m_Buffer);
		}

		bool IsBufferChanged()
		{
			return m_Buffer != GetValueFromData();
		}

	protected:
		T m_Buffer;
	};

	class BoolDataWidget : public BasicDataWidgetT<bool>
	{
	public:
		BoolDataWidget(const DataSpec& dataSpec);

		virtual void Draw(const entt::meta_any& compInstance, int32_t elementIndex = -1) override;
	};

	template<typename T, uint32_t N = 1, typename CT = T>
	class ScalarNDataWidget : public BasicDataWidgetT<T>
	{
	public:
		ScalarNDataWidget(const DataSpec& dataSpec, ImGuiDataType scalarType, CT defaultMin, CT defaultMax, const char* format)
			: m_ScalarType(scalarType)
			, m_DefaultMin(defaultMin), m_DefaultMax(defaultMax)
			, m_Format(format)
		{
			static_assert(N == 1 || N == 2 || N == 3, "N can only be 1, 2 or 3!");

			Init(dataSpec);
		}

		virtual void Draw(const entt::meta_any& compInstance, int32_t elementIndex = -1) override
		{
			if (!PreDraw(compInstance, elementIndex)) return;

			const auto data = m_DataSpec.Data;
			const auto dragSpeed = GetPropValue<float>(PropertyType::DragSensitivity, data);
			const float dragSpeedValue = dragSpeed.value_or(1.0f);
			const auto min = GetPropValue<CT>(PropertyType::ClampMin, data);
			const auto minValue = min.value_or(m_DefaultMin);
			const auto max = GetPropValue<CT>(PropertyType::ClampMax, data);
			const auto maxValue = max.value_or(m_DefaultMax);
			const ImGuiSliderFlags clampMode = DoesPropExist(PropertyType::ClampOnlyDuringDragging, data) ? 0 : ImGuiSliderFlags_AlwaysClamp;
			void* valuePtr = nullptr;
			if constexpr (N == 1) // C++17 constexpr if
			{
				valuePtr = &m_Buffer;
			}
			else
			{
				valuePtr = glm::value_ptr(m_Buffer);
			}

			bool bChanged = ImGui::DragScalarNEx("", m_ScalarType, valuePtr, N, dragSpeedValue, &minValue, &maxValue, m_Format, clampMode);
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

	private:
		ImGuiDataType m_ScalarType;
		CT m_DefaultMin, m_DefaultMax;
		const char* m_Format;
	};

	class EnumDataWidget : public BasicDataWidgetT<entt::meta_any>
	{
	public:
		EnumDataWidget(const DataSpec& dataSpec);

		virtual void Draw(const entt::meta_any& compInstance, int32_t elementIndex = -1) override;

	protected:
		void InitEnumDatas();

		virtual void UpdateBuffer() override;

		virtual entt::meta_any GetValueFromData() override
		{
			return m_DataSpec.GetValue();
		}

	private:
		const char* m_CurrentEnumDataName = nullptr;
		std::list<entt::meta_data> m_EnumDatas;
	};

	class StringDataWidget : public BasicDataWidgetT<std::string>
	{
	public:
		StringDataWidget(const DataSpec& dataSpec);

		virtual void Draw(const entt::meta_any& compInstance, int32_t elementIndex = -1) override;
	};

	class ColorDataWidget : public BasicDataWidgetT<glm::vec4>
	{
	public:
		ColorDataWidget(const DataSpec& dataSpec);

		virtual void Draw(const entt::meta_any& compInstance, int32_t elementIndex = -1) override;
	};

	class Texture2DDataWidget : public BasicDataWidgetT<Ref<Texture2D>>
	{
	public:
		Texture2DDataWidget(const DataSpec& dataSpec);

		virtual void Draw(const entt::meta_any& compInstance, int32_t elementIndex = -1) override;
	};

	class ContainerWidget : public DataWidget
	{
	public:
		virtual bool PreDraw(const entt::meta_any& compInstance, int32_t elementIndex = -1) override;
		virtual void PostDraw() override;

	private:
		virtual void DrawContainerOperationWidget() = 0;
	};

	class SequenceContainerWidget : public ContainerWidget
	{
	public:
		SequenceContainerWidget(const DataSpec& dataSpec);

		virtual void Draw(const entt::meta_any& compInstance, int32_t elementIndex = -1) override;

	private:
		virtual void DrawContainerOperationWidget() override;
		void DrawContainerElementOperationWidget(entt::meta_sequence_container::iterator& it);
		entt::meta_sequence_container::iterator InsertValue(entt::meta_sequence_container::iterator it);
		entt::meta_sequence_container::iterator EraseValue(entt::meta_sequence_container::iterator it);

	private:
		Ref<DataWidget> m_ElementWidgetTemplate;
	};

	class AssociativeContainerWidget : public ContainerWidget
	{
	public:
		AssociativeContainerWidget(const DataSpec& dataSpec);

		virtual void Draw(const entt::meta_any& compInstance, int32_t elementIndex = -1) override;

	private:
		virtual void DrawContainerOperationWidget() override;
		void DrawContainerElementOperationWidget(entt::meta_associative_container::iterator& it);
		entt::meta_associative_container::iterator InsertValue(entt::meta_associative_container::iterator it);
		entt::meta_associative_container::iterator EraseValue(entt::meta_associative_container::iterator it);

	};

}
