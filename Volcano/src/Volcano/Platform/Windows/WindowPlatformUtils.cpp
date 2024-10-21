#include "volpch.h"
#include "Volcano/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <shlobj.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Volcano/Core/Application.h"
#include <tchar.h>
#include "Volcano/Utils/FileUtils.h"

namespace Volcano {

    std::filesystem::path FileDialogs::s_ProjectPath;

    float Time::GetTime()
    {
        return glfwGetTime();
    }

    std::string FileDialogs::OpenFile(const char* filter, std::string oldPath)
    {
        OPENFILENAME ofn;                        // common dialog box structure  文件夹对话框
        CHAR szFile[260] = { 0 };                // 在Windows平台的C++编程中，LPWSTR是指向Unicode宽字符（UTF-16编码）字符串的指针类型，通常用于Windows API函数。
        ZeroMemory(&ofn, sizeof(OPENFILENAME));  // 用0来填充一块内存区域。
        ofn.lStructSize = sizeof(OPENFILENAME);
        // 父窗口是谁
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        if (!oldPath.empty())
        {
            auto lastDot = oldPath.rfind('.');
            bool directory = lastDot == std::string::npos;
            if (directory)
            {
                FileUtils::CreatePath(oldPath);
                ofn.lpstrInitialDir = (LPSTR)oldPath.data();
            }
            else if (std::filesystem::exists(oldPath))
                ofn.lpstrInitialDir = (LPSTR)oldPath.data();
        }
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameA(&ofn) == TRUE) {
            return ofn.lpstrFile;
        }
        return std::string();
    }

    int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
    {
        if (uMsg == BFFM_INITIALIZED)
        {
            //(LPARAM)"D:\\remote"--指定初始目录  BFFM_SETSELECTION（指向目录）
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        }
        return 0;
    }

    std::string FileDialogs::OpenFolder(std::string oldPath)
    {
        std::string newPath(oldPath);
        std::filesystem::path pathBuffer(newPath);
        while (!std::filesystem::exists(pathBuffer))
            pathBuffer = pathBuffer.parent_path();
        newPath = pathBuffer.string();

        char path[MAX_PATH];
        BROWSEINFO bi = { 0 };
        bi.lpszTitle = _T("请选择文件夹:");
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
        //bi.pszDisplayName = (LPSTR)oldPath.c_str();
        bi.lpfn = BrowseCallbackProc; // 文件夹对话框初始化时选中特定目录
        bi.lParam = (LPARAM)newPath.c_str(); // 传递初始路径

        LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
        if (pidl == nullptr) {
            // 文件夹浏览框点击取消，返回oldPath
            return oldPath;
        }
        if (!SHGetPathFromIDList(pidl, path)) {
            return oldPath;
        }

        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }
        return path;
    }

    std::string FileDialogs::SaveFile(const char* filter, std::string oldPath)
    {
        OPENFILENAME ofn;
        CHAR szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        if (!oldPath.empty())
        {
            auto lastDot = oldPath.rfind('.');
            bool directory = lastDot == std::string::npos;
            if (directory)
            {
                FileUtils::CreatePath(oldPath);
                ofn.lpstrInitialDir = (LPSTR)oldPath.data();
            }
            else if (std::filesystem::exists(oldPath))
                ofn.lpstrInitialDir = (LPSTR)oldPath.data();
        }
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetSaveFileNameA(&ofn) == TRUE) {
            return ofn.lpstrFile;
        }
        return std::string();

    }

    std::filesystem::path FileDialogs::GetProjectPath()
    {
        if (s_ProjectPath.empty())
        {
            char path[MAX_PATH];
            if (GetModuleFileName(NULL, path, MAX_PATH) == 0) {
                VOL_ERROR("Failed to get exe path.");
                return s_ProjectPath;
            }
            else
                s_ProjectPath = std::filesystem::u8path(path).parent_path();
        }
        return s_ProjectPath;

    }
}