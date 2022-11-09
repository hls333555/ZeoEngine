#include "ZEpch.h"
#include "Engine/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <shellapi.h> // NOTE: Include required windows.h before it

#include "Engine/Core/Application.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Utils/PathUtils.h"

namespace ZeoEngine {

	std::vector<std::string> FileDialogs::Open(bool bAllowMultiSelect)
	{
		std::vector<std::string> outPaths;

		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()));
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		const std::string filterStr = GetSupportedFileFilter();
		ofn.lpstrFilter = filterStr.c_str(); // NOTE: We must first store the returned string first to extend its lifetime
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER | (bAllowMultiSelect ? OFN_ALLOWMULTISELECT : 0);
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			auto p = ofn.lpstrFile;
			const std::string basePath = p;
			p += basePath.size() + 1;
			if (*p == 0)
			{
				// There is only one string, being the full path to the file
				outPaths.emplace_back(basePath.c_str());
			}
			else
			{
				// Multiple files follow the directory
				while (*p != 0)
				{
					std::string fileName = p;
					outPaths.emplace_back((basePath + "\\" + fileName).c_str());
					p += fileName.size() + 1;
				}
			}
		}
		return outPaths;
	}

	std::optional<std::string> FileDialogs::Save()
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()));
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		std::string filterStr = GetSupportedFileFilter();
		ofn.lpstrFilter = filterStr.c_str();
		ofn.nFilterIndex = 1;
		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = strchr(ofn.lpstrFilter, '\0') + 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return {};
	}

	std::string FileDialogs::GetSupportedFileFilter()
	{
		std::string extensionStr;
		for (auto it = AssetManager::Get().m_SupportedFileExtensions.begin();;)
		{
			extensionStr += it->first;
			if (++it != AssetManager::Get().m_SupportedFileExtensions.end())
			{
				extensionStr += ";*";
			}
			else
			{
				break;
			}
		}
		std::stringstream ss;
		ss << "All supported Files (*" << extensionStr << ")" << '\0' << "*" << extensionStr << '\0'; // NOTE: The \0 must be char instead of string!
		return ss.str();
	}

	void PlatformUtils::ShowInExplorer(const std::string& filepath)
	{
		std::wstringstream params;
		params << "/select," << PathUtils::GetCanonicalPath(filepath).c_str(); // Use canonical path here, path separator must be "\\"
		ShellExecute(NULL, L"open", L"explorer.exe", params.str().c_str(), NULL, SW_SHOWDEFAULT);
	}

	void PlatformUtils::OpenFile(const std::string& filepath)
	{
		std::string path = PathUtils::GetCanonicalPath(filepath); // Use canonical path here, path separator must be "\\"
		ShellExecute(0, 0, std::wstring(path.begin(), path.end()).c_str(), 0, 0, SW_SHOW);
	}

}
