#include "ptpch.h"
#include "Proton\Utils\PlatformUtils.h"

#include <commdlg.h>

#include "Proton\Core\Application.h"
#include "Platform\Windows\WindowsWindow.h"
#include "Platform\Windows\WindowsInput.h"

namespace Proton
{
	std::string FileDialogs::OpenFile(const char* filter)
	{
		OPENFILENAMEA openFileName;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&openFileName, sizeof(OPENFILENAME));
		openFileName.lStructSize = sizeof(OPENFILENAME);
		openFileName.hwndOwner = (HWND)Application::Get().GetWindow().GetNativeWindow();
		openFileName.lpstrFile = szFile;
		openFileName.nMaxFile = sizeof(szFile);
		openFileName.lpstrFilter = filter;
		openFileName.nFilterIndex = 1;
		openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		static_cast<WindowsWindow&>(Application::Get().GetWindow()).ClearKeys();

		if (GetOpenFileNameA(&openFileName) == TRUE)
		{
			return openFileName.lpstrFile;
		}

		return std::string();
	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		OPENFILENAMEA openFileName;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&openFileName, sizeof(OPENFILENAME));
		openFileName.lStructSize = sizeof(OPENFILENAME);
		openFileName.hwndOwner = (HWND)Application::Get().GetWindow().GetNativeWindow();
		openFileName.lpstrFile = szFile;
		openFileName.nMaxFile = sizeof(szFile);
		openFileName.lpstrFilter = filter;
		openFileName.nFilterIndex = 1;
		openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		static_cast<WindowsWindow&>(Application::Get().GetWindow()).ClearKeys();

		if (GetSaveFileNameA(&openFileName) == TRUE)
		{
			return openFileName.lpstrFile;
		}

		return std::string();
	}
}