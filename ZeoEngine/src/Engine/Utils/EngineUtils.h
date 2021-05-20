#pragma once

namespace ZeoEngine {

	/** Returns absolute path. */
	std::string GetCanonicalPath(const std::string& path);
	/** Returns relative path. */
	std::string GetRelativePath(const std::string& path);
	/** Returns file name including file extension. */
	std::string GetFileNameFromPath(const std::string& path);
	/** Returns file name excluding file extension. */
	std::string GetNameFromPath(const std::string& path);
	/** Returns file extension. */
	std::string GetExtensionFromPath(const std::string& path);

}
