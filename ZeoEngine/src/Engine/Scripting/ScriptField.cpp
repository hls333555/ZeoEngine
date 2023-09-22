#include "ZEpch.h"
#include "Engine/Scripting/ScriptField.h"

#include <mono/metadata/reflection.h>

#include "Engine/Scripting/ScriptEngine.h"

namespace ZeoEngine {

	bool ScriptField::HasAttribute(const std::string& name) const
	{
		auto* parentClass = mono_field_get_parent(ClassField);
		auto* attrInfo = mono_custom_attrs_from_field(parentClass, ClassField);
		if (!attrInfo) return false;

		auto* attrClass = mono_class_from_name(ScriptEngine::GetCoreAssemblyImage(), "ZeoEngine.Attributes", name.c_str());
		if (!attrClass) return false;

		return mono_custom_attrs_has_attr(attrInfo, attrClass);
	}

	bool ScriptField::GetAttributeValueInternal(const std::string& name, void* outValue) const
	{
		auto* parentClass = mono_field_get_parent(ClassField);
		auto* attrInfo = mono_custom_attrs_from_field(parentClass, ClassField);
		if (!attrInfo) return false;

		auto* attrClass = mono_class_from_name(ScriptEngine::GetCoreAssemblyImage(), "ZeoEngine.Attributes", name.c_str());
		if (!attrClass) return false;

		if (!mono_custom_attrs_has_attr(attrInfo, attrClass)) return false;

		auto* attrObj = mono_custom_attrs_get_attr(attrInfo, attrClass);
		auto* valueField = mono_class_get_field_from_name(attrClass, "m_Value");
		ZE_CORE_ASSERT(valueField, "Attribute class {} does not have field: 'm_Value'!", name);
		mono_field_get_value(attrObj, valueField, outValue);
		mono_custom_attrs_free(attrInfo);
		return true;
	}

	bool ScriptField::GetAttributeValueInternal(const std::string& name, std::string& outValue) const
	{
		MonoString* monoStr = nullptr;
		const bool res = GetAttributeValueInternal(name, &monoStr);
		if (!res) return false;

		char* str = mono_string_to_utf8(monoStr);
		outValue = monoStr != nullptr ? str : "";
		mono_free(str);
		return true;
	}
	
}
