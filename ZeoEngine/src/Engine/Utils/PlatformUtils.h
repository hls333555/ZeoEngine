#pragma once

#include <optional>
#include <string>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class FileDialogs
	{
	public:
		// These return empty strings if cancelled
		static std::optional<std::string> Open();
		static std::optional<std::string> Save();

	private:
		static std::string GetSupportedFileFilter();
	};

	class PlatformUtils
	{
	public:
		/** Open the file explorer and select the path. */
		static void ShowInExplorer(const std::string& path);

		/** Open the file using the default program. */
		static void OpenFile(const std::string& path);
	};

}
