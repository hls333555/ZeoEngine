#pragma once

#include <rttr/type>
#define RAPIDJSON_HAS_STDSTRING 1
#include <prettywriter.h> // for stringify JSON
#include <document.h>     // rapidjson's DOM-style API

namespace ZeoEngine {

	class Serializer
	{
		using ObjectInstantiationFn = std::function<rttr::variant()>;

	public:
		Serializer(const Serializer&) = delete;
		Serializer& operator=(const Serializer&) = delete;

		static Serializer& Get()
		{
			static Serializer serializer;
			return serializer;
		}

	private:
		Serializer() = default;

	public:
		/**
		 * Serialize an object into a string, then you can output that string into a file.
		 * In most cases, you should stream out file type first, which will be checked against duing file validation stage when deserializing.
		 */
		std::string Serialize(const rttr::instance& object);

		/**
		 * Returns true if file validation succeeds.
		 * @param validationStr - string to validate, currently should be type of this file (e.g."Level", "ParticleSystem")
		 * @param OutProcessedStr - [OUT] string ready to deserialize, if validation failed, this will be an empty string
		 */
		bool ValidateFile(const std::string& filePath, const std::string& validationStr, std::string& outProcessedStr);
		
		/**
		 * Deserialize a string.
		 * NOTE: Only call this if ValidateFile() returns true!
		 * @templ T - Type of instantiated object, should be a pointer
		 * @param instantiator - Function that performs instantiation of an object to be deserialized to
		 */
		template<typename T>
		bool Deserialize(const std::string& extractedString, ObjectInstantiationFn instantiator)
		{
			// Default template parameter uses UTF8 and MemoryPoolAllocator
			rapidjson::Document document;
			// "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream()
			if (document.Parse(extractedString).HasParseError())
			{
				ZE_CORE_ERROR("Error parsing serialization file!");
				return false;
			}

			rttr::variant& object = instantiator();
			DeserializeRecursively(object, document);
			// Call the callback
			object.get_value<T>()->OnDeserialized();
			return true;
		}

	private:
		void SerializeRecursively(const rttr::instance& object, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
		bool SerializeValue(const rttr::variant& var, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
		bool SerializeAtomicTypes(const rttr::type& type, const rttr::variant& var, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
		void SerializeSequentialContainerTypes(const rttr::variant_sequential_view& sequentialView, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);
		void SerializeAssociativeContainerTypes(const rttr::variant_associative_view& associativeView, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);

		void DeserializeRecursively(const rttr::instance& object, rapidjson::Value& jsonObject);
		rttr::variant DeserializeBasicTypes(rapidjson::Value& jsonValue);
		void DeserializeSequentialContainerTypes(rttr::variant_sequential_view& sequentialView, rapidjson::Value& jsonArrayValue);
		void DeserializeAssociativeContainerTypes(rttr::variant_associative_view& associativeView, rapidjson::Value& jsonArrayValue);
		rttr::variant ExtractValue(rapidjson::Value::MemberIterator& it, const rttr::type& type);

	};

}
