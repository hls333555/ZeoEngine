#pragma once

#include "Engine/Core/Buffer.h"

namespace ZeoEngine {

	class FileSystemUtils
	{
	public:
		/** Returns true if path exists. */
		static bool Exists(const std::string& path);
		/** Returns true if path is a directory. */
		static bool IsDirectory(const std::string& path);
		/** Returns parent path. */
		static std::string GetParentPath(const std::string& path);
		/** Returns file name excluding file extension. */
		static std::string GetPathName(const std::string& path);
		/** Returns file name including file extension. */
		static std::string GetPathFileName(const std::string& path);
		/** Returns file extension. */
		static std::string GetPathExtension(const std::string& path);
		/** Returns normalized absolute path. Especially useful for platform specific path. */
		static std::string GetCanonicalPath(const std::string& path);
		/** Returns the virtual path widely used in engine. The virtual path looks like "Game/Folder/Asset" or "Engine/Folder/Asset". */
		static std::string GetStandardPath(const std::filesystem::path& path);
		/** Returns the actual path. The actual path can be relative or absolute. */
		static std::string GetFileSystemPath(const std::string& path);

		/** Create directories recursively and returns true if succeeded. */
		static bool CreateDirectory(const std::string& directory);
		/** Rename a path. */
		static void RenamePath(const std::string& oldPath, const std::string& newPath);
		/** Remove a path (possibly its sub-paths). Returns true if succeeded. */
		static bool DeletePath(const std::string& path);
		/** Copy a file to another place. Returns true if succeeded. */
		static bool CopyFile(const std::string& srcPath, const std::string& destPath, bool bShouldOverwrite);
		/** Read file as bytes into buffer and return it.  */
		static Buffer ReadFileBinary(const std::string& path);

		static bool CopyAsset(const std::string& srcPath, const std::string& destPath);

	};

}
