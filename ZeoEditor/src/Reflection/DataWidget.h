#pragma once

#include <string>
#include <deque>

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <doctest.h>

#include "Engine/Core/Core.h"
#include "Engine/Core/ReflectionHelper.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/GameFramework/Components.h"
#include "Reflection/DataParser.h"
#include "Test/TestComponent.h"
#include "Reflection/AssetBrowser.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	template<typename T>
	void ShowPropertyTooltip(T metaObj)
	{
		if (ImGui::IsItemHovered())
		{
			auto tooltip = GetPropValue<const char*>(PropertyType::Tooltip, metaObj);
			if (tooltip)
			{
				ImGui::SetTooltipWithPadding(*tooltip);
			}
		}
	}

	uint32_t GetAggregatedDataID(entt::meta_data data);

	Ref<class DataWidget> ConstructBasicDataWidget(DataSpec& dataSpec, entt::meta_type type, bool bIsTest = false);

	static const ImGuiTreeNodeFlags DefaultDataTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
	static const ImGuiTreeNodeFlags EmptyContainerDataTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
	static const ImGuiTreeNodeFlags DefaultContainerDataTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
	static const ImGuiTreeNodeFlags DefaultStructDataTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

	class DataWidget
	{
	public:
		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) = 0;

#ifndef DOCTEST_CONFIG_DISABLE
		void Test(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex = -1);
	private:
		virtual void TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex) {}
#endif

	protected:
		// Call this at the beginning of drawing!
		virtual bool PreDraw(entt::meta_any& compInstance, entt::meta_any& instance) = 0;
		// Call this in the end of drawing!
		virtual void PostDraw() = 0;

		void Init(DataSpec& dataSpec, bool bIsTest);

		void InvokeOnDataValueEditChangeCallback(entt::meta_data data, std::any oldValue);
		void InvokePostDataValueEditChangeCallback(entt::meta_data data, std::any oldValue);

	protected:
		DataSpec m_DataSpec;
		bool m_bIsTest = false;
	};

	template<typename T>
	class BasicDataWidgetT : public DataWidget
	{
	protected:
		virtual bool PreDraw(entt::meta_any& compInstance, entt::meta_any& instance) override
		{
			m_DataSpec.Update(compInstance, instance);
			UpdateBuffer();
			m_OldBuffer = m_Buffer;

			if (m_DataSpec.bIsSeqElement) return true;

			ImGui::AlignTextToFramePadding();
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

		virtual void UpdateBuffer()
		{
			m_Buffer = GetValueFromData();
		}

		T GetValueFromData()
		{
			return m_DataSpec.GetValue<T>();
		}

#ifndef DOCTEST_CONFIG_DISABLE
		T GetTestDataValue(entt::registry& reg, entt::entity entity, const std::vector<DataStackSpec>& dataStack, int32_t elementIndex)
		{
			auto compInstance = Reflection::GetComponent(entt::resolve(entt::type_hash<TestComponent>::value()), reg, entity);
			entt::meta_any instance = compInstance.as_ref();
			// Recursively get parent instance
			for (const auto& spec : dataStack)
			{
				instance = spec.bIsSeqElement ? spec.Data.get(instance).as_sequence_container()[spec.ElementIndex] : spec.Data.get(instance);
			}
			if (m_DataSpec.bIsSeqElement)
			{
				auto seqView = m_DataSpec.Data.get(instance).as_sequence_container();
				instance = seqView[elementIndex];
			}
			return Reflection::GetDataValue<T>(m_DataSpec.Data, instance, m_DataSpec.bIsSeqElement);
		}
#endif

		void SetValueToData(bool bShouldCallPostCallback = true)
		{
			m_DataSpec.SetValue(m_Buffer);
			if (bShouldCallPostCallback)
			{
				InvokePostDataValueEditChangeCallback(m_DataSpec.Data, m_OldBuffer);
			}
			else
			{
				InvokeOnDataValueEditChangeCallback(m_DataSpec.Data, m_OldBuffer);
			}
		}

		bool IsBufferChanged()
		{
			return m_Buffer != GetValueFromData();
		}

	protected:
		T m_Buffer, m_OldBuffer;
	};

	class BoolDataWidget : public BasicDataWidgetT<bool>
	{
	public:
		BoolDataWidget(DataSpec& dataSpec, bool bIsTest);

		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) override;

	private:
#ifndef DOCTEST_CONFIG_DISABLE
		virtual void TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex) override;
#endif
	};

	template<typename T, uint32_t N = 1, typename CT = T>
	class ScalarNDataWidget : public BasicDataWidgetT<T>
	{
	public:
		ScalarNDataWidget(DataSpec& dataSpec, bool bIsTest, ImGuiDataType scalarType, CT defaultMin, CT defaultMax, const char* format)
			: m_ScalarType(scalarType)
			, m_DefaultMin(defaultMin), m_DefaultMax(defaultMax)
			, m_Format(format)
		{
			static_assert(N == 1 || N == 2 || N == 3, "N can only be 1, 2 or 3!");

			Init(dataSpec, bIsTest);
		}

		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) override
		{
			if (!PreDraw(compInstance, instance)) return;

			auto data = m_DataSpec.Data;
			const auto dragSpeed = GetPropValue<float>(PropertyType::DragSensitivity, data);
			const float dragSpeedValue = dragSpeed.value_or(1.0f);
			const auto min = GetPropValue<CT>(PropertyType::ClampMin, data);
			const auto minValue = min.value_or(m_DefaultMin);
			const auto max = GetPropValue<CT>(PropertyType::ClampMax, data);
			const auto maxValue = max.value_or(m_DefaultMax);
			const ImGuiSliderFlags clampMode = DoesPropExist(PropertyType::ClampOnlyDuringDragging, data) ? 0 : ImGuiSliderFlags_AlwaysClamp;
			void* valuePtr = nullptr;
			if constexpr (N == 1)
			{
				valuePtr = &m_Buffer;
			}
			else
			{
				valuePtr = glm::value_ptr(m_Buffer);
			}

			bool bChanged = ImGui::DragScalarNEx("", m_ScalarType, valuePtr, N, dragSpeedValue, &minValue, &maxValue, m_Format, clampMode);
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

	private:
#ifndef DOCTEST_CONFIG_DISABLE
		virtual void TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex) override
		{
			m_Buffer = T(10.0);
			SetValueToData();
			CHECK(GetTestDataValue(reg, entity, dataStack, elementIndex) == m_Buffer);
		}
#endif

	private:
		ImGuiDataType m_ScalarType;
		CT m_DefaultMin, m_DefaultMax;
		const char* m_Format;
	};

	class EnumDataWidget : public BasicDataWidgetT<entt::meta_any>
	{
	public:
		EnumDataWidget(DataSpec& dataSpec, bool bIsTest);

		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) override;

	protected:
		void InitEnumDatas();

		virtual void UpdateBuffer() override;

	private:
#ifndef DOCTEST_CONFIG_DISABLE
		virtual void TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex) override;
#endif

	private:
		const char* m_CurrentEnumDataName = nullptr;
		std::deque<entt::meta_data> m_EnumDatas;
	};

	class StringDataWidget : public BasicDataWidgetT<std::string>
	{
	public:
		StringDataWidget(DataSpec& dataSpec, bool bIsTest);

		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) override;

	private:
#ifndef DOCTEST_CONFIG_DISABLE
		virtual void TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex) override;
#endif
	};

	class ColorDataWidget : public BasicDataWidgetT<glm::vec4>
	{
	public:
		ColorDataWidget(DataSpec& dataSpec, bool bIsTest);

		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) override;

	private:
#ifndef DOCTEST_CONFIG_DISABLE
		virtual void TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex) override;
#endif
	};

	class Texture2DDataWidget : public BasicDataWidgetT<AssetHandle<Texture2DAsset>>
	{
	public:
		Texture2DDataWidget(DataSpec& dataSpec, bool bIsTest);

		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) override;

	private:
#ifndef DOCTEST_CONFIG_DISABLE
		virtual void TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex) override;
#endif

	private:
		AssetBrowser m_Browser{ Texture2DAsset::TypeId() };
	};

	class ParticleTemplateDataWidget : public BasicDataWidgetT<AssetHandle<ParticleTemplateAsset>>
	{
	public:
		ParticleTemplateDataWidget(DataSpec& dataSpec, bool bIsTest);

		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) override;

	private:
#ifndef DOCTEST_CONFIG_DISABLE
		virtual void TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex) override;
#endif

	private:
		AssetBrowser m_Browser{ ParticleTemplateAsset::TypeId() };
	};

	class ContainerWidget : public DataWidget
	{
	protected:
		virtual bool PreDraw(entt::meta_any& compInstance, entt::meta_any& instance) override;
		virtual void PostDraw() override;

	private:
		virtual void DrawContainerOperationWidget() = 0;
	};

	class SequenceContainerWidget : public ContainerWidget
	{
	public:
		SequenceContainerWidget(DataSpec& dataSpec, bool bIsTest);

		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) override;

	private:
#ifndef DOCTEST_CONFIG_DISABLE
		virtual void TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex) override;
#endif

	private:
		virtual void DrawContainerOperationWidget() override;
		void DrawContainerElementOperationWidget(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator& it);
		entt::meta_sequence_container::iterator InsertValue(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it);
		entt::meta_sequence_container::iterator EraseValue(entt::meta_sequence_container& seqView, entt::meta_sequence_container::iterator it);

	private:
		Ref<DataWidget> m_ElementWidgetTemplate;
	};

	class AssociativeContainerWidget : public ContainerWidget
	{
	public:
		AssociativeContainerWidget(DataSpec& dataSpec, bool bIsTest);

		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) override;

	private:
		virtual void DrawContainerOperationWidget() override;
		void DrawContainerElementOperationWidget(entt::meta_associative_container& assView, entt::meta_associative_container::iterator& it);
		entt::meta_associative_container::iterator InsertValue(entt::meta_associative_container& assView, entt::meta_associative_container::iterator it);
		entt::meta_associative_container::iterator EraseValue(entt::meta_associative_container& assView, entt::meta_associative_container::iterator it);

	};

	class StructWidget : public DataWidget
	{
	public:
		StructWidget(DataSpec& dataSpec, bool bIsTest);

		virtual void Draw(entt::meta_any& compInstance, entt::meta_any& instance) override;

	private:
#ifndef DOCTEST_CONFIG_DISABLE
		virtual void TestImpl(entt::registry& reg, entt::entity entity, std::vector<DataStackSpec>& dataStack, int32_t elementIndex) override;
#endif

	protected:
		virtual bool PreDraw(entt::meta_any& compInstance, entt::meta_any& instance) override;
		virtual void PostDraw() override;

	private:
		void PreprocessStruct(entt::meta_type structType);
		void PreprocessSubdata(entt::meta_data subdata);
		void DrawSubdataWidget(entt::meta_data subdata, entt::meta_any& structInstance);

	private:
		DataParser m_SubdataParser;

		/** Map from aggregated subdata id to DataWidget instance */
		std::unordered_map<uint32_t, Ref<DataWidget>> m_SubdataWidgets;

		/** List of all its subdata, used to draw ordered registered subdatas */
		std::deque<uint32_t> m_PreprocessedSubdatas;
		bool m_bIsPreprocessedSubdatasDirty = true;
	};

}
