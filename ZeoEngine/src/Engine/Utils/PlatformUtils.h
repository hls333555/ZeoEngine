#pragma once

#include <optional>
#include <string>

namespace ZeoEngine {

	class FileDialogs
	{
	public:
		static std::vector<std::string> Open(bool bAllowMultiSelect);
		static std::optional<std::string> Save();

	private:
		static std::string GetSupportedFileFilter();
	};

	class PlatformUtils
	{
	public:
		/** Open the file explorer and select the path. */
		static void ShowInExplorer(const std::string& filepath);

		/** Open the file using the default program. */
		static void OpenFile(const std::string& filepath);
	};

}
