#pragma once

namespace ZeoEngine {

	class PathUtils
	{
	public:
		/** Returns true if path exists. */
		static bool Exists(const std::filesystem::path& path);
		/** Returns true if path is a directory. */
		static bool IsDirectory(const std::filesystem::path& path);
		/**
		 * Compares two paths. Returns true if they are the same.
		 * NOTE: This comparison reuqires two path to exist.
		 */
		static bool Equivalent(const std::filesystem::path& lPath, const std::filesystem::path& rPath);

		/** Create directories recursively and returns true if succeeded. */
		static bool CreateDirectory(const std::filesystem::path& directory);
		/** Rename a path. */
		static void RenamePath(const std::filesystem::path& oldPath, const std::filesystem::path& newPath);
		/** Remove a path (possibly its sub-paths). Returns true if succeeded. */
		static bool DeletePath(const std::filesystem::path& path);
		/** Copy a file to another place. Returns true if succeeded. */
		static bool CopyFile(const std::filesystem::path& srcPath, const std::filesystem::path& destPath, bool bShouldOverwrite);

		static bool CopyAsset(const std::filesystem::path& srcPath, const std::filesystem::path& destPath);

	};

}
