#pragma once

#include <optional>
#include <string>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class FileDialogs
	{
	public:
		// These return empty strings if cancelled
		static std::optional<std::string> OpenFile();
		static std::optional<std::string> SaveFile();

	private:
		static std::string GetSupportedFileFilter();
	};

	class PlatformUtils
	{
	public:
		/** Open the file explorer and select the path. */
		static void ShowInExplorer(const std::string& path);
	};

}
