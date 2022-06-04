#pragma once

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class PathUtils
	{
	public:
		/** Returns canonical absolute path. */
		static std::string GetCanonicalPath(const std::string& path);
		/** Returns relative path. */
		static std::string GetRelativePath(const std::string& path);
		/** Returns parent path. */
		static std::string GetParentPath(const std::string& path);

		/**
		 * Compares two paths. Returns true if they are the same.
		 * NOTE: This comparison reuqires two path to exist.
		 */
		static bool ArePathsEquivalent(const std::string& lPath, const std::string& rPath);

		/** Returns file name including file extension. */
		static std::string GetFileNameFromPath(const std::string& path);
		/** Returns file name excluding file extension. */
		static std::string GetNameFromPath(const std::string& path);
		/** Returns file extension. */
		static std::string GetExtensionFromPath(const std::string& path);

		/** Append a path to the base path. Returns the appended path. */
		[[nodiscard]] static std::string AppendPath(const std::string& basePath, const std::string& appendPath);

		/** Returns true if path exists. */
		static bool DoesPathExist(const std::string& path);

		/** Create a direct directory and returns true if succeeded. */
		static bool CreateDirectory(const std::string& directory);
		/** Create a directory recursively and returns true if succeeded. */
		static bool CreateDirectories(const std::string& directory);
		/** Rename a path. */
		static void RenamePath(const std::string& oldPath, const std::string& newPath);
		/** Remove a path (and its sub-paths). Returns number of paths removed. */
		static uintmax_t DeletePath(const std::string& path, bool bRecursively = true);
		/** Copy a file to another place. Returns false if failed. */
		static bool CopyFile(const std::string& srcPath, const std::string& destPath, bool bShouldOverwrite);

		/**
		 * Iterate through the path's parent path.
		 * The signature of the func must be equivalent to the following form:
		 * 
		 * @code:
		 * bool(const std::string& name); // Returns true to break the loop
		 * @endcode
		 * 
		 */
		template<typename Func>
		static void ForEachParentPathInPath(const std::string& path, Func func)
		{
			std::filesystem::path currentPath{ path };
			for (auto parentPath : currentPath)
			{
				if (func(parentPath.string())) break;
			}
		}

		/** Get resource path by erasing asset extension if necessary. */
		static std::string GetResourcePathFromPath(const std::string& path);
		/** Get asset path by appending asset extension if necessary. */
		static std::string GetNormalizedAssetPath(const std::string& path);

	};

}
