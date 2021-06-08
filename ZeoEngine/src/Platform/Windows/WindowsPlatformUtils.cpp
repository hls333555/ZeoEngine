#include "ZEpch.h"
#include "Engine/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Engine/Core/Application.h"
#include "Engine/Core/AssetManager.h"

namespace ZeoEngine {

	std::optional<std::string> FileDialogs::OpenFile()
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()));
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		std::string filterStr = GetSupportedAssetFilter();
		ofn.lpstrFilter = filterStr.c_str(); // NOTE: We must first store the returned string first to extend its lifetime
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return {};
	}

	std::optional<std::string> FileDialogs::SaveFile()
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()));
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		std::string filterStr = GetSupportedAssetFilter();
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

	std::string FileDialogs::GetSupportedAssetFilter()
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

}
