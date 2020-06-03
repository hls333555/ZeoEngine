#include "ZEpch.h"
#include "Engine/Core/Serializer.h"

#include "Engine/GameFramework/GameObject.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Core/EngineGlobals.h"

namespace ZeoEngine {

	//////////////////////////////////////////////////////////////////////////
	// Serialization /////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	std::string Serializer::Serialize(const rttr::instance& object)
	{
		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);

		SerializeRecursively(object, writer);
		return sb.GetString();
	}

	void Serializer::SerializeRecursively(const rttr::instance& object, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
	{
		writer.StartObject();
		rttr::instance obj = object.get_type().get_raw_type().is_wrapper() ? object.get_wrapped_instance() : object;
		const auto& properties = obj.get_derived_type().get_properties();
		for (auto prop : properties)
		{
			if (prop.get_metadata(PropertyMeta::Transient))
				continue;

			rttr::variant var = prop.get_value(obj);
			// Cannot serialize, because we cannot retrieve the value
			if (!var)
				continue;

			const auto name = prop.get_name();
			writer.String(name.data(), static_cast<rapidjson::SizeType>(name.length()), false);
			if (!SerializeValue(var, writer))
			{
				ZE_CORE_ERROR("Failed to serialize property: {0}", name);
			}
		}
		writer.EndObject();
	}

	bool Serializer::SerializeValue(const rttr::variant& var, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
	{
		rttr::type valueType = var.get_type();
		rttr::type wrappedType = valueType.is_wrapper() ? valueType.get_wrapped_type() : valueType;
		bool is_wrapper = wrappedType != valueType;
		if (SerializeAtomicTypes(wrappedType, is_wrapper ? var.extract_wrapped_value() : var, writer))
		{
		}
		else if (var.is_sequential_container())
		{
			SerializeSequentialContainerTypes(var.create_sequential_view(), writer);
		}
		else if (var.is_associative_container())
		{
			SerializeAssociativeContainerTypes(var.create_associative_view(), writer);
		}
		else
		{
			auto& childProps = is_wrapper ? wrappedType.get_properties() : valueType.get_properties();
			if (!childProps.empty())
			{
				SerializeRecursively(var, writer);
			}
			else
			{
				bool bOk = false;
				std::string stringValue = var.to_string(&bOk);
				if (!bOk)
				{
					writer.String(stringValue);
					return false;
				}
				writer.String(stringValue);
			}
		}
		return true;
	}

	bool Serializer::SerializeAtomicTypes(const rttr::type& type, const rttr::variant& var, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
	{
		if (type.is_arithmetic())
		{
			if (type == rttr::type::get<bool>())
			{
				writer.Bool(var.to_bool());
			}
			else if (type == rttr::type::get<char>())
			{
				writer.Bool(var.to_bool());
			}
			else if (type == rttr::type::get<int8_t>())
			{
				writer.Int(var.to_int8());
			}
			else if (type == rttr::type::get<int16_t>())
			{
				writer.Int(var.to_int16());
			}
			else if (type == rttr::type::get<int32_t>())
			{
				writer.Int(var.to_int32());
			}
			else if (type == rttr::type::get<int64_t>())
			{
				writer.Int64(var.to_int64());
			}
			else if (type == rttr::type::get<uint8_t>())
			{
				writer.Uint(var.to_uint8());
			}
			else if (type == rttr::type::get<uint16_t>())
			{
				writer.Uint(var.to_uint16());
			}
			else if (type == rttr::type::get<uint32_t>())
			{
				writer.Uint(var.to_uint32());
			}
			else if (type == rttr::type::get<uint64_t>())
			{
				writer.Uint64(var.to_uint64());
			}
			else if (type == rttr::type::get<float>())
			{
				writer.Double(var.to_double());
			}
			else if (type == rttr::type::get<double>())
			{
				writer.Double(var.to_double());
			}
			return true;
		}
		// enum
		else if (type.is_enumeration())
		{
			bool bOk = false;
			const std::string stringValue = var.to_string(&bOk);
			if (bOk)
			{
				writer.String(stringValue);
			}
			else
			{
				bOk = false;
				uint64_t uint64Value = var.to_uint64(&bOk);
				if (bOk)
				{
					writer.Uint64(uint64Value);
				}
				else
				{
					writer.Null();
				}
			}
			return true;
		}
		// std::string
		else if (type.get_raw_type() == rttr::type::get<std::string>())
		{
			if (type.is_pointer())
			{
				writer.String(*var.get_value<std::string*>());
			}
			else
			{
				writer.String(var.to_string());
			}
			return true;
		}
		// glm::i32vec2
		else if (type.get_raw_type() == rttr::type::get<glm::i32vec2>())
		{
			if (type.is_pointer())
			{
				writer.StartArray();
				writer.Int(var.get_value<glm::i32vec2*>()->x);
				writer.Int(var.get_value<glm::i32vec2*>()->y);
				writer.EndArray();
			}
			else
			{
				writer.StartArray();
				writer.Int(var.get_value<glm::i32vec2>().x);
				writer.Int(var.get_value<glm::i32vec2>().y);
				writer.EndArray();
			}
			return true;
		}
		// glm::vec2
		else if (type.get_raw_type() == rttr::type::get<glm::vec2>())
		{
			if (type.is_pointer())
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec2*>()->x);
				writer.Double(var.get_value<glm::vec2*>()->y);
				writer.EndArray();
			}
			else
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec2>().x);
				writer.Double(var.get_value<glm::vec2>().y);
				writer.EndArray();
			}
			return true;
		}
		// glm::vec3
		else if (type.get_raw_type() == rttr::type::get<glm::vec3>())
		{
			if (type.is_pointer())
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec3*>()->x);
				writer.Double(var.get_value<glm::vec3*>()->y);
				writer.Double(var.get_value<glm::vec3*>()->z);
				writer.EndArray();
			}
			else
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec3>().x);
				writer.Double(var.get_value<glm::vec3>().y);
				writer.Double(var.get_value<glm::vec3>().z);
				writer.EndArray();
			}
			return true;
		}
		// glm::vec4
		else if (type.get_raw_type() == rttr::type::get<glm::vec4>())
		{
			if (type.is_pointer())
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec4*>()->x);
				writer.Double(var.get_value<glm::vec4*>()->y);
				writer.Double(var.get_value<glm::vec4*>()->z);
				writer.Double(var.get_value<glm::vec4*>()->w);
				writer.EndArray();
			}
			else
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec4>().x);
				writer.Double(var.get_value<glm::vec4>().y);
				writer.Double(var.get_value<glm::vec4>().z);
				writer.Double(var.get_value<glm::vec4>().w);
				writer.EndArray();
			}
			return true;
		}
		// TODO: Serialize GameObject*
		// GameObject*
		else if (type.is_pointer() && type == rttr::type::get<GameObject*>())
		{
			ZE_CORE_WARN("Serializing GameObject* is currently not supported!");
			writer.String(var.get_value<GameObject*>()->GetUniqueName());
			return true;
		}
		// Ref<Texture2D>
		else if (type.get_raw_type() == rttr::type::get<Texture2D>())
		{
			const auto& texture = var.get_value<Ref<Texture2D>>();
			writer.String(texture ? texture->GetPath() : "");
			return true;
		}
		// ParticleSystem*
		else if (type.is_pointer() && type == rttr::type::get<ParticleSystem*>())
		{
			ParticleSystem* ps = var.get_value<ParticleSystem*>();
			writer.String(ps ? ps->GetPath() : "");
			return true;
		}
		return false;
	}

	void Serializer::SerializeSequentialContainerTypes(const rttr::variant_sequential_view& sequentialView, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
	{
		writer.StartArray();
		for (const auto& item : sequentialView)
		{
			if (item.is_sequential_container())
			{
				SerializeSequentialContainerTypes(item.create_sequential_view(), writer);
			}
			else
			{
				rttr::variant wrappedVar = item.extract_wrapped_value();
				rttr::type valueType = wrappedVar.get_type();
				if (SerializeAtomicTypes(valueType, wrappedVar, writer))
				{
				}
				else
				{
					SerializeRecursively(wrappedVar, writer);
				}
			}
		}
		writer.EndArray();
	}

	void Serializer::SerializeAssociativeContainerTypes(const rttr::variant_associative_view& associativeView, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
	{
		static const std::string keyName("key");
		static const std::string valueName("value");

		writer.StartArray();
		if (associativeView.is_key_only_type())
		{
			for (auto& item : associativeView)
			{
				SerializeValue(item.first, writer);
			}
		}
		else
		{
			for (auto& item : associativeView)
			{
				writer.StartObject();
				writer.String(keyName.c_str(), static_cast<rapidjson::SizeType>(keyName.size()), false);
				SerializeValue(item.first, writer);
				writer.String(valueName.c_str(), static_cast<rapidjson::SizeType>(valueName.size()), false);
				SerializeValue(item.second, writer);
				writer.EndObject();
			}
		}
		writer.EndArray();
	}

	//////////////////////////////////////////////////////////////////////////
	// Deserialization ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Serializer::ValidateFile(const std::string& filePath, const std::string& validationStr, std::string& outProcessedStr)
	{
		std::ifstream in(filePath, std::ios::in | std::ios::binary);
		if (in)
		{
			std::string firstLine;
			// Type token is supposed to be the FIRST line
			std::getline(in, firstLine);
			if (firstLine.empty())
			{
				ZE_CORE_ERROR("Validation failed: Unrecognized {0} file format!", validationStr.c_str());
				return false;
			}

			const char* typeToken = "#type";
			size_t typeTokenLength = strlen(typeToken);
			// Location of file type
			size_t typeTokenPos = firstLine.find(typeToken, 0);
			if (typeTokenPos == std::string::npos)
			{
				ZE_CORE_ERROR("Validation failed: Unrecognized {0} file format!", validationStr.c_str());
				return false;
			}

			// End of line
			size_t eol = firstLine.find_first_of("\r\n", typeTokenPos);
			size_t typePos = typeTokenPos + typeTokenLength + 1;
			// Get file type
			std::string type = firstLine.substr(typePos, eol - typePos);
			if (type != validationStr)
			{
				ZE_CORE_ERROR("Validation failed: Unrecognized {0} file format!", validationStr.c_str());
				return false;
			}

			in.seekg(0, std::ios::end);
			auto size = in.tellg();
			if (size == -1)
			{
				ZE_CORE_ERROR("Validation failed: Could not read from file {0}!", filePath);
				return false;
			}
			else
			{
				outProcessedStr.resize(size);
				// "1" should be '\n'
				in.seekg(firstLine.size() + 1, std::ios::beg);
				// Read from second line till end of file
				in.read(&outProcessedStr[0], size);
				in.close();
			}
		}
		else
		{
			ZE_CORE_ERROR("Validation failed: Could not open file {0}!", filePath);
			return false;
		}

		return true;
	}

	void Serializer::DeserializeRecursively(const rttr::instance& object, rapidjson::Value& jsonObject)
	{
		rttr::instance obj = object.get_type().get_raw_type().is_wrapper() ? object.get_wrapped_instance() : object;
		const auto& properties = obj.get_derived_type().get_properties();
		for (auto prop : properties)
		{
			auto it = jsonObject.FindMember(prop.get_name().data());
			if (it == jsonObject.MemberEnd())
				continue;

			const rttr::type type = prop.get_type();
			auto& jsonValue = it->value;
			switch (jsonValue.GetType())
			{
				case rapidjson::kArrayType:
				{
					rttr::variant var = prop.get_value(obj);
					// Note: type.is_sequential_container() will not work properly
					if (var.is_sequential_container())
					{
						DeserializeSequentialContainerTypes(var.create_sequential_view(), jsonValue);
					}
					else if (var.is_associative_container())
					{
						DeserializeAssociativeContainerTypes(var.create_associative_view(), jsonValue);
					}
					// These vectors are stored as json arrays
					else if (type.get_raw_type() == rttr::type::get<glm::i32vec2>())
					{
						if (type.is_pointer())
						{
							var.get_value<glm::i32vec2*>()->x = jsonValue[0].GetInt();
							var.get_value<glm::i32vec2*>()->y = jsonValue[1].GetInt();
						}
						else
						{
							var.get_value<glm::i32vec2>().x = jsonValue[0].GetInt();
							var.get_value<glm::i32vec2>().y = jsonValue[1].GetInt();
						}
					}
					else if (type.get_raw_type() == rttr::type::get<glm::vec2>())
					{
						if (type.is_pointer())
						{
							var.get_value<glm::vec2*>()->x = static_cast<float>(jsonValue[0].GetDouble());
							var.get_value<glm::vec2*>()->y = static_cast<float>(jsonValue[1].GetDouble());
						}
						else
						{
							var.get_value<glm::vec2>().x = static_cast<float>(jsonValue[0].GetDouble());
							var.get_value<glm::vec2>().y = static_cast<float>(jsonValue[1].GetDouble());
						}
					}
					else if (type.get_raw_type() == rttr::type::get<glm::vec3>())
					{
						if (type.is_pointer())
						{
							var.get_value<glm::vec3*>()->x = static_cast<float>(jsonValue[0].GetDouble());
							var.get_value<glm::vec3*>()->y = static_cast<float>(jsonValue[1].GetDouble());
							var.get_value<glm::vec3*>()->z = static_cast<float>(jsonValue[2].GetDouble());
						}
						else
						{
							var.get_value<glm::vec3>().x = static_cast<float>(jsonValue[0].GetDouble());
							var.get_value<glm::vec3>().y = static_cast<float>(jsonValue[1].GetDouble());
							var.get_value<glm::vec3>().z = static_cast<float>(jsonValue[2].GetDouble());
						}
					}
					else if (type.get_raw_type() == rttr::type::get<glm::vec4>())
					{
						if (type.is_pointer())
						{
							var.get_value<glm::vec4*>()->x = static_cast<float>(jsonValue[0].GetDouble());
							var.get_value<glm::vec4*>()->y = static_cast<float>(jsonValue[1].GetDouble());
							var.get_value<glm::vec4*>()->z = static_cast<float>(jsonValue[2].GetDouble());
							var.get_value<glm::vec4*>()->w = static_cast<float>(jsonValue[3].GetDouble());
						}
						else
						{
							var.get_value<glm::vec4>().x = static_cast<float>(jsonValue[0].GetDouble());
							var.get_value<glm::vec4>().y = static_cast<float>(jsonValue[1].GetDouble());
							var.get_value<glm::vec4>().z = static_cast<float>(jsonValue[2].GetDouble());
							var.get_value<glm::vec4>().w = static_cast<float>(jsonValue[3].GetDouble());
						}
					}
					prop.set_value(obj, var);
					break;
				}
				case rapidjson::kObjectType:
				{
					rttr::variant var = prop.get_value(obj);
					DeserializeRecursively(var, jsonValue);
					prop.set_value(obj, var);
					break;
				}
				default:
				{
					rttr::variant extractedValue = DeserializeBasicTypes(jsonValue);
					// TODO: CONVERSION WORKS ONLY WITH "const type", check whether this is correct or not!
					if (extractedValue.convert(type))
					{
						prop.set_value(obj, extractedValue);
					}
					else if (type == rttr::type::get<std::string*>())
					{
						*prop.get_value(obj).get_value<std::string*>() = extractedValue.to_string();
					}
					else if (type.get_raw_type() == rttr::type::get<Ref<Texture2D>>())
					{
						std::string extractedPath = extractedValue.to_string();
						if (!extractedPath.empty())
						{
							prop.set_value(obj, GetTexture2DLibrary()->GetOrLoad(extractedPath));
						}
					}
					else if (type.is_pointer() && type == rttr::type::get<ParticleSystem*>())
					{
						std::string extractedPath = extractedValue.to_string();
						if (!extractedPath.empty())
						{
							prop.set_value(obj, GetParticleLibrary()->GetOrLoad(extractedPath));
						}
					}
					else
					{
						ZE_CORE_ERROR("Failed to deserialize property: {0}!", prop.get_name());
					}
				}
			}
		}
	}

	rttr::variant Serializer::DeserializeBasicTypes(rapidjson::Value& jsonValue)
	{
		switch (jsonValue.GetType())
		{
		case rapidjson::kNullType:
		{
			break;
		}
		case rapidjson::kFalseType:
		case rapidjson::kTrueType:
		{
			return jsonValue.GetBool();
		}
		case rapidjson::kNumberType:
		{
			if (jsonValue.IsInt())
			{
				return jsonValue.GetInt();
			}
			else if (jsonValue.IsInt64())
			{
				return jsonValue.GetInt64();
			}
			else if (jsonValue.IsUint())
			{
				return jsonValue.GetUint();
			}
			else if (jsonValue.IsUint64())
			{
				return jsonValue.GetUint64();
			}
			else if (jsonValue.IsDouble())
			{
				return jsonValue.GetDouble();
			}
			break;
		}
		case rapidjson::kStringType:
		{
			return std::string(jsonValue.GetString());
		}
		// We handle only the basic types here
		case rapidjson::kObjectType:
		case rapidjson::kArrayType:
		{
			return rttr::variant();
		}
		}

		return rttr::variant();
	}

	void Serializer::DeserializeSequentialContainerTypes(rttr::variant_sequential_view& sequentialView, rapidjson::Value& jsonArrayValue)
	{
		sequentialView.set_size(jsonArrayValue.Size());
		const rttr::type arrayValueType = sequentialView.get_rank_type(1);
		for (rapidjson::SizeType i = 0; i < jsonArrayValue.Size(); ++i)
		{
			auto& jsonIndexValue = jsonArrayValue[i];
			if (jsonIndexValue.IsArray())
			{
				auto& subSequentialView = sequentialView.get_value(i).create_sequential_view();
				DeserializeSequentialContainerTypes(subSequentialView, jsonIndexValue);
			}
			else if (jsonIndexValue.IsObject())
			{
				rttr::variant var = sequentialView.get_value(i);
				rttr::variant wrappedVar = var.extract_wrapped_value();
				DeserializeRecursively(wrappedVar, jsonIndexValue);
				sequentialView.set_value(i, wrappedVar);
			}
			else
			{
				rttr::variant extractedValue = DeserializeBasicTypes(jsonIndexValue);
				if (extractedValue.convert(arrayValueType))
				{
					sequentialView.set_value(i, extractedValue);
				}
			}
		}
	}

	void Serializer::DeserializeAssociativeContainerTypes(rttr::variant_associative_view& associativeView, rapidjson::Value& jsonArrayValue)
	{
		for (rapidjson::SizeType i = 0; i < jsonArrayValue.Size(); ++i)
		{
			auto& jsonIndexValue = jsonArrayValue[i];
			// A key-value associative view
			if (jsonIndexValue.IsObject())
			{
				auto keyIt = jsonIndexValue.FindMember("key");
				auto valueIt = jsonIndexValue.FindMember("value");

				if (keyIt != jsonIndexValue.MemberEnd() && valueIt != jsonIndexValue.MemberEnd())
				{
					rttr::variant keyVar = ExtractValue(keyIt, associativeView.get_key_type());
					rttr::variant valueVar = ExtractValue(valueIt, associativeView.get_value_type());
					if (keyVar && valueVar)
					{
						associativeView.insert(keyVar, valueVar);
					}
				}
			}
			// A key-only associative view
			else
			{
				rttr::variant extractedValue = DeserializeBasicTypes(jsonIndexValue);
				if (extractedValue && extractedValue.convert(associativeView.get_key_type()))
				{
					associativeView.insert(extractedValue);
				}
			}
		}
	}
	// TODO: ExtractValue
	rttr::variant Serializer::ExtractValue(rapidjson::Value::MemberIterator& it, const rttr::type& type)
	{
		auto& jsonValue = it->value;
		rttr::variant extractedValue = DeserializeBasicTypes(jsonValue);
		const bool bCanConvert = extractedValue.convert(type);
		if (!bCanConvert)
		{
			if (jsonValue.IsObject())
			{
				rttr::constructor ctor = type.get_constructor();
				for (auto& item : type.get_constructors())
				{
					if (item.get_instantiated_type() == type)
					{
						ctor = item;
					}
				}
				extractedValue = ctor.invoke();
				DeserializeRecursively(extractedValue, jsonValue);
			}
		}

		return extractedValue;
	}

}
