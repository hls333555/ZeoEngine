#pragma once

#include <entt.hpp>
#include <mono/metadata/reflection.h>

#include "Engine/Core/ReflectionCore.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/Scripting/ScriptRegistry.h"

namespace ZeoEngine {

	using namespace entt::literals;

	template<typename Component>
	class RegisterComponent
	{
	public:
		template<typename... Property>
		RegisterComponent(const char* name, Property... properties)
			: m_MetaFactory(entt::meta<Component>().type().props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...))
		{
			static_assert(std::is_base_of_v<IComponent, Component>, "Registered Component should derive from 'IComponent'!");

			m_MetaFactory.base<IComponent>()
				.ctor<&Reflection::emplace<Component>, entt::as_ref_t>()
				.func<&Reflection::remove<Component>, entt::as_void_t>("remove"_hs)
				.func<&Reflection::get<Component>, entt::as_ref_t>("get"_hs)
				.func<&Reflection::has<Component>>("has"_hs)
				.func<&Reflection::copy<Component>, entt::as_ref_t>("copy"_hs)
				.func<&Reflection::bind_on_destroy<Component>, entt::as_void_t>("bind_on_destroy"_hs)
				.func<&Component::GetIcon>("get_icon"_hs);

			if (auto* monoType = mono_reflection_type_from_name(GetMonoComponentName<Component>().data(), ScriptEngine::GetCoreAssemblyImage()))
			{
				ScriptRegistry::s_RegisteredMonoComponents[monoType] = entt::type_hash<Component>::value();
			}
		}

		template<auto Data, typename... Property>
		RegisterComponent& Data(const char* name, Property... properties)
		{
			m_MetaFactory.data<Data, entt::as_ref_t>(entt::hashed_string(name)).props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...);
			return *this;
		}

		template<auto Setter, auto Getter, typename... Property>
		RegisterComponent& Data(const char* name, Property... properties)
		{
			if constexpr (std::is_same_v<decltype(Setter), std::nullptr_t>)
			{
				m_MetaFactory.data<Setter, Getter, entt::as_ref_t>(entt::hashed_string(name)).props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...);
			}
			else
			{
				m_MetaFactory.data<Setter, Getter, entt::as_is_t>(entt::hashed_string(name)).props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...);
			}
			return *this;
		}

	private:
		template<typename T>
		std::string GetMonoComponentName()
		{
			auto view = entt::type_name<T>::value();
			auto separator = view.find("::");
			return fmt::format("{}.{}", view.substr(0, separator), view.substr(separator + 2, view.size() - separator));
		}

	private:
		entt::meta_factory<Component> m_MetaFactory;
	};

	template<typename Enum>
	class RegisterEnum
	{
	public:
		RegisterEnum()
			: m_MetaFactory(entt::meta<Enum>())
		{
			m_MetaFactory.func<&Reflection::set_enum_value_for_seq<Enum>, entt::as_void_t>("set_enum_value_for_seq"_hs);
		}

		template<auto Data, typename... Property>
		RegisterEnum& Data(const char* name, Property... properties)
		{
			m_MetaFactory.data<Data>(entt::hashed_string(name)).props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...);
			return *this;
		}

	private:
		entt::meta_factory<Enum> m_MetaFactory;
	};

	/**
	 * Register a struct contained in a component.
	 * The struct MUST have == operator overloading like below:
	 * 
	 * @code
	 * bool operator==(const MyStruct& other) const; // <- This "const" is required!
	 * @endcode
	 */
	template<typename Struct>
	class RegisterStruct
	{
	public:
		template<typename... Property>
		RegisterStruct(const char* name, Property... properties)
			: m_MetaFactory(entt::meta<Struct>().type().props(std::make_pair(Reflection::Name, name), Reflection::Inherent, Reflection::Struct, std::forward<Property>(properties)...))
		{
		}

		template<auto Data, typename... Property>
		RegisterStruct& Data(const char* name, Property... properties)
		{
			m_MetaFactory.data<Data, entt::as_ref_t>(entt::hashed_string(name)).props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...);
			return *this;
		}

		template<auto Setter, auto Getter, typename... Property>
		RegisterStruct& Data(const char* name, Property... properties)
		{
			m_MetaFactory.data<Setter, Getter, entt::as_is_t>(entt::hashed_string(name)).props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...);
			return *this;
		}

	private:
		entt::meta_factory<Struct> m_MetaFactory;
	};

	class TypeRegistry
	{
	public:
		static void Init();
		static void RegisterBasicTypes();
		static void RegisterComponents();
	};

}
