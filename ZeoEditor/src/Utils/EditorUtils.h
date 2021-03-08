#pragma once

#include <string>

#include <magic_enum.hpp>

namespace ZeoEngine {

	template<typename T>
	std::string ResolveEditorNameFromEnum(T type)
	{
		std::string name = magic_enum::enum_name(type).data();
		for (std::string::size_type pos{ 0 }; pos != std::string::npos; ++pos) {
			if ((pos = name.find("_", pos)) == std::string::npos) break;

			name.replace(pos, 1, " ");
		}
		return name;
	}

}
