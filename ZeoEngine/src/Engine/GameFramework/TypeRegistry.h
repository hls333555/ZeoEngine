#pragma once

#include <entt.hpp>

#include "Engine/Core/ReflectionCore.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Scripting/ScriptRegistry.h"

namespace ZeoEngine {

	using namespace entt::literals;

	template<typename Component, typename BaseComponent = IComponent>
	class RegisterComponent
	{
	public:
		template<typename... Property>
		RegisterComponent(const char* name, Property... properties)
			: m_MetaFactory(entt::meta<Component>().type().props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...))
		{
			static_assert(std::is_base_of_v<IComponent, Component>, "Registered Component should derive from 'IComponent'!");

			m_MetaFactory.base<BaseComponent>()
				.ctor<&Reflection::emplace<Component>, entt::as_ref_t>()
				.func<&Reflection::remove<Component>, entt::as_void_t>("remove"_hs)
				.func<&Reflection::get<Component>, entt::as_ref_t>("get"_hs)
				.func<&Reflection::has<Component>>("has"_hs)
				.func<&Reflection::patch<Component>, entt::as_void_t>("patch"_hs)
				.func<&Reflection::copy<Component>, entt::as_ref_t>("copy"_hs)
				.func<&Component::GetIcon>("get_icon"_hs);

			ScriptRegistry::RegisterMonoComponent(GetMonoComponentName().data(), entt::type_hash<Component>::value());
		}

		template<auto Field, typename... Property>
		RegisterComponent& Field(const char* name, Property... properties)
		{
			m_MetaFactory.data<Field, entt::as_ref_t>(entt::hashed_string(name)).props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...);
			return *this;
		}

		template<bool IsGetter, auto Getter, typename... Property>
		RegisterComponent& Field(const char* name, Property... properties)
		{
			m_MetaFactory.data<nullptr, Getter, entt::as_ref_t>(entt::hashed_string(name)).props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...);
			return *this;
		}

	private:
		std::string GetMonoComponentName()
		{
			auto view = entt::type_name<Component>::value();
			auto separator = view.find("::");
			SizeT startPos = 7; // Excluding "struct "
			return fmt::format("{}.{}", view.substr(startPos, separator - startPos), view.substr(separator + 2, view.size() - separator));
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
		}

		template<auto Field, typename... Property>
		RegisterEnum& Field(const char* name, Property... properties)
		{
			m_MetaFactory.data<Field>(entt::hashed_string(name)).props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...);
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

		template<auto Field, typename... Property>
		RegisterStruct& Field(const char* name, Property... properties)
		{
			m_MetaFactory.data<Field, entt::as_ref_t>(entt::hashed_string(name)).props(std::make_pair(Reflection::Name, name), std::forward<Property>(properties)...);
			return *this;
		}

		template<auto Setter, auto Getter, typename... Property>
		RegisterStruct& Field(const char* name, Property... properties)
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

	private:
		static void RegisterBasicTypes();
		static void RegisterComponents();
		static void RegisterComponentSerializerExtenders();
	};

}
