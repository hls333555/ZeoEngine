#include "Inspectors/FieldWidget.h"
#include "Engine/ImGui/ScriptClassBrowser.h"
#include "Engine/Physics/PhysicsTypes.h"
#include "Engine/ImGui/CollisionLayerBrowser.h"

namespace ZeoEngine {

	template<typename FieldInstance>
	class ScriptClassFieldWidget : public FieldWidgetBase<FieldInstance>
	{
	public:
		using FieldWidgetBase<FieldInstance>::FieldWidgetBase;

	private:
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			auto value = fieldInstance->GetValue<std::string>();
			float rightPadding = 0.0f;
			if constexpr (Utils::IsFieldSequenceContainer<FieldInstance>())
			{
				rightPadding = Utils::GetContainerDropdownWidth();
			}
			if (m_Browser.Draw(value, rightPadding))
			{
				this->ApplyValueToInstance(&value);
			}
		}

		virtual void ApplyValueToInstance(const void* value) override
		{
			const auto fieldInstance = this->GetFieldInstance();
			const auto oldValueStr = fieldInstance->GetValue<std::string>(); // Copy string
			const auto& valueStr = *static_cast<const std::string*>(value);
			if (valueStr == oldValueStr) return;

			fieldInstance->SetValue(valueStr);

			const U32 fieldID = fieldInstance->GetFieldID();
			fieldInstance->OnFieldValueChanged(fieldID);
		}

	private:
		ScriptClassBrowser m_Browser;
	};

	template<typename FieldInstance>
	class LockFlagsFieldWidget : public FieldWidgetBase<FieldInstance>
	{
	public:
		using FieldWidgetBase<FieldInstance>::FieldWidgetBase;

	private:
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			U8 value = fieldInstance->GetValue<U8>();
			bool bIsTranslationXLocked = static_cast<U8>(ActorLockFlag::TranslationX) & value;
			bool bIsTranslationYLocked = static_cast<U8>(ActorLockFlag::TranslationY) & value;
			bool bIsTranslationZLocked = static_cast<U8>(ActorLockFlag::TranslationZ) & value;
			bool bIsRotationXLocked = static_cast<U8>(ActorLockFlag::RotationX) & value;
			bool bIsRotationYLocked = static_cast<U8>(ActorLockFlag::RotationY) & value;
			bool bIsRotationZLocked = static_cast<U8>(ActorLockFlag::RotationZ) & value;

			ImGui::TableNextColumn();

			ImGui::Text("		Translation");

			ImGui::TableNextColumn();

			if (ImGui::Checkbox("X##Translation", &bIsTranslationXLocked))
			{
				if (bIsTranslationXLocked)
				{
					value |= static_cast<U8>(ActorLockFlag::TranslationX);
				}
				else
				{
					value &= ~static_cast<U8>(ActorLockFlag::TranslationX);
				}
				this->ApplyValueToInstance(&value);
			}

			ImGui::SameLine();

			if (ImGui::Checkbox("Y##Translation", &bIsTranslationYLocked))
			{
				if (bIsTranslationYLocked)
				{
					value |= static_cast<U8>(ActorLockFlag::TranslationY);
				}
				else
				{
					value &= ~static_cast<U8>(ActorLockFlag::TranslationY);
				}
				this->ApplyValueToInstance(&value);
			}

			ImGui::SameLine();

			if (ImGui::Checkbox("Z##Translation", &bIsTranslationZLocked))
			{
				if (bIsTranslationZLocked)
				{
					value |= static_cast<U8>(ActorLockFlag::TranslationZ);
				}
				else
				{
					value &= ~static_cast<U8>(ActorLockFlag::TranslationZ);
				}
				this->ApplyValueToInstance(&value);
			}

			ImGui::TableNextColumn();

			ImGui::Text("		Rotation");

			ImGui::TableNextColumn();

			if (ImGui::Checkbox("X##Rotation", &bIsRotationXLocked))
			{
				if (bIsRotationXLocked)
				{
					value |= static_cast<U8>(ActorLockFlag::RotationX);
				}
				else
				{
					value &= ~static_cast<U8>(ActorLockFlag::RotationX);
				}
				this->ApplyValueToInstance(&value);
			}

			ImGui::SameLine();

			if (ImGui::Checkbox("Y##Rotation", &bIsRotationYLocked))
			{
				if (bIsRotationYLocked)
				{
					value |= static_cast<U8>(ActorLockFlag::RotationY);
				}
				else
				{
					value &= ~static_cast<U8>(ActorLockFlag::RotationY);
				}
				this->ApplyValueToInstance(&value);
			}

			ImGui::SameLine();

			if (ImGui::Checkbox("Z##Rotation", &bIsRotationZLocked))
			{
				if (bIsRotationZLocked)
				{
					value |= static_cast<U8>(ActorLockFlag::RotationZ);
				}
				else
				{
					value &= ~static_cast<U8>(ActorLockFlag::RotationZ);
				}
				this->ApplyValueToInstance(&value);
			}
		}

		virtual void SetValue(const void* value) override
		{
			auto v = *static_cast<const U8*>(value);
			this->GetFieldInstance()->SetValue(v);
		}
	};

	template<typename FieldInstance, typename Browser>
	class CollisionLayerFieldWidget : public FieldWidgetBase<FieldInstance>
	{
	public:
		using FieldWidgetBase<FieldInstance>::FieldWidgetBase;

	private:
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			auto value = fieldInstance->GetValue<U32>();
			float rightPadding = 0.0f;
			if constexpr (Utils::IsFieldSequenceContainer<FieldInstance>())
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
			auto v = *static_cast<const U32*>(value);
			this->GetFieldInstance()->SetValue(v);
		}

	private:
		Browser m_Browser;
	};

	Scope<IFieldWidget> ConstructScriptClassFieldWidget(U32 widgetID, Ref<ComponentFieldInstance> fieldInstance)
	{
		return CreateScope<ScriptClassFieldWidget<ComponentFieldInstance>>(widgetID, std::move(fieldInstance));
	}

	Scope<IFieldWidget> ConstructLockFlagsFieldWidget(U32 widgetID, Ref<ComponentFieldInstance> fieldInstance)
	{
		return CreateScope<LockFlagsFieldWidget<ComponentFieldInstance>>(widgetID, std::move(fieldInstance));
	}

	Scope<IFieldWidget> ConstructCollisionLayerFieldWidget(U32 widgetID, Ref<ComponentFieldInstance> fieldInstance)
	{
		return CreateScope<CollisionLayerFieldWidget<ComponentFieldInstance, CollisionLayerBrowser>>(widgetID, std::move(fieldInstance));
	}

	Scope<IFieldWidget> ConstructCollisionGroupFieldWidget(U32 widgetID, Ref<ComponentFieldInstance> fieldInstance)
	{
		return CreateScope<CollisionLayerFieldWidget<ComponentFieldInstance, CollisionGroupBrowser>>(widgetID, std::move(fieldInstance));
	}
	
}
