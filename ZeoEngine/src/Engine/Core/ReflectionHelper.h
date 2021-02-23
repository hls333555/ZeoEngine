#pragma once

#include <any>
#include <optional>

#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

	enum class BasicMetaType
	{
		NONE,
		STRUCT, // Custom struct
		SEQCON, // Sequence container
		ASSCON, // Associative container
		BOOL, I8, I32, I64, UI8, UI32, UI64,
		FLOAT, DOUBLE,
		ENUM,
		STRING, VEC2, VEC3, VEC4, TEXTURE, PARTICLE
	};

	template<typename T>
	std::optional<const char*> GetMetaObjectDisplayName(T metaObj)
	{
		auto prop = metaObj.prop(PropertyType::DisplayName);
		if (prop)
		{
			return prop.value().cast<const char*>();
		}
		else
		{
			prop = metaObj.prop(PropertyType::Name);
			if (prop)
			{
				return prop.value().cast<const char*>();
			}
		}
		return {};
	}

	BasicMetaType EvaluateMetaType(const entt::meta_type type);

	struct DataSpec
	{
	public:
		const char* DataName;
		entt::meta_data Data;
		entt::meta_any ParentInstance, ComponentInstance;

		bool bIsStructSubdata;

		int32_t ElementIndex;

		bool bIsSeqElement;
		entt::meta_sequence_container SeqView;

		DataSpec() = default;
		DataSpec(entt::meta_data data, const entt::meta_any& compInstance, const entt::meta_any& parentInstance, bool isStructSubdata, bool isSeqElement)
			: Data(data)
			, ComponentInstance(compInstance)
			, ParentInstance(parentInstance)
			, bIsStructSubdata(isStructSubdata)
			, bIsSeqElement(isSeqElement)
		{
			const auto dataName = GetMetaObjectDisplayName(Data);
			DataName = *dataName;
			TryUpdateSeqView();
		}

		void Update(const entt::meta_any& compInstance, const entt::meta_any& parentInstance, int32_t elementIndex)
		{
			// NOTE: After this (meta_any) assignment, "compInstance" may be invalid! Use "ComponentInstance" instead!
			ComponentInstance = compInstance;
			ParentInstance = parentInstance;
			TryUpdateSeqView();
			ElementIndex = elementIndex;
		}

		entt::meta_type GetType() const
		{
			if (bIsSeqElement) return SeqView.value_type();
			return Data.type();
		}

		size_t GetContainerSize() const
		{
			if (SeqView) return SeqView.size();

			return 0;
		}

		entt::meta_any GetValue()
		{
			if (bIsSeqElement)
			{
				return SeqView[ElementIndex];
			}
			else
			{
				return Data.get(ParentInstance);
			}
		}

		template<typename T>
		T GetValue()
		{
			if (bIsSeqElement)
			{
				return SeqView[ElementIndex].cast<T>();
			}
			else
			{
				return Data.get(ParentInstance).cast<T>();
			}
		}

		template<typename T>
		void SetValue(T&& value)
		{
			if (bIsSeqElement)
			{
				SeqView[ElementIndex].cast<T>() = value;
			}
			else
			{
				Data.set(ParentInstance, std::forward<T>(value));
			}
		}

	private:
		void TryUpdateSeqView()
		{
			if (Data.type().is_sequence_container())
			{
				SeqView = Data.get(ParentInstance).as_sequence_container();
			}
		}
	};

	namespace Reflection {

		void RemoveComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		entt::meta_any GetComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		entt::meta_any HasComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);

		void BindOnDestroy(entt::meta_type compType, entt::registry& registry);

		void SetEnumValueForSeq(entt::meta_any& instance, entt::meta_any& newValue);

	}
	
	const char* GetEnumDisplayName(entt::meta_any enumValue);

	// TODO: Remove
	entt::meta_any CreateTypeDefaultValue(entt::meta_type type);

	template<typename T>
	bool IsTypeEqual(entt::meta_type type)
	{
		return type.info().hash() == entt::type_hash<T>::value();
	}

	// TODO: Remove
	template<typename T>
	T GetDataValue(entt::meta_data data, entt::meta_handle instance)
	{
		return data.get(std::move(instance)).cast<T>();
	}

	// TODO: Remove
	template<typename T>
	void SetDataValue(entt::meta_data data, entt::meta_handle instance, T&& value)
	{
		data.set(std::move(instance), std::forward<T>(value));
	}

	template<typename T>
	bool DoesPropExist(PropertyType propType, T metaObj)
	{
		return static_cast<bool>(metaObj.prop(propType));
	}

	template<typename Ret, typename T>
	std::optional<Ret> GetPropValue(PropertyType propType, T metaObj)
	{
		const auto prop = metaObj.prop(propType);
		if (prop)
		{
			const auto value = prop.value();
			// As int32_t conversions are already registered, the system can convert to all registered integral types seamlessly
			return value.allow_cast<Ret>().cast<Ret>();
		}
		return {};
	}

}
