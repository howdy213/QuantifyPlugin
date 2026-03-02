/**
 * @file virtualkeyboard.h
 * @brief 班级记录类
 * @author howdy213
 * @date 2026-3-1
 * @version 1.3.0
 *
 * Copyright (C) 2025-2026 howdy213
 *
 * This file is part of QuantifyPlugin.
 *
 * QuantifyPlugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuantifyPlugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <Objbase.h>
#include <Psapi.h>
#include <QDir>
#include <QVector>
#include <TlHelp32.h>
#include <initguid.h>
#include <iostream>
#include <memory>
#include <objbase.h>
#include <windows.h>
typedef LONG(WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

#include "WECore/WFile/wshellexecute.h"
#include "virtualkeyboard.h"

struct ComDeleter {
    void operator()(IUnknown *ptr) const {
        if (ptr)
            ptr->Release();
    }
};

template <typename T> using com_ptr = std::unique_ptr<T, ComDeleter>;

/// \brief 获取指定进程名的所有 PID
/// \param fileName
/// \return
///
QList<DWORD> GetProcessIDs(const QString &fileName) {
    QList<DWORD> pids;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return pids;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (fileName.compare(QString::fromWCharArray(pe.szExeFile),
                                 Qt::CaseInsensitive) == 0)
                pids.append(pe.th32ProcessID);
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return pids;
}

// 4ce576fa-83dc-4F88-951c-9d0782b4e376
DEFINE_GUID(CLSID_UIHostNoLaunch, 0x4CE576FA, 0x83DC, 0x4f88, 0x95, 0x1C, 0x9D,
            0x07, 0x82, 0xB4, 0xE3, 0x76);

// 37c994e7_432b_4834_a2f7_dce1f13b834b
DEFINE_GUID(IID_ITipInvocation, 0x37c994e7, 0x432b, 0x4834, 0xa2, 0xf7, 0xdc,
            0xe1, 0xf1, 0x3b, 0x83, 0x4b);

struct ITipInvocation : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Toggle(HWND wnd) = 0;
};

namespace {
const char *KeyboardWindowClass = "IPTip_Main_Window";
const char *WindowParentClass = "ApplicationFrameWindow";
const char *WindowClass = "Windows.UI.Core.CoreWindow";
const char *WindowCaption = "Microsoft Text Input Application";
} // namespace

///
/// \brief VirtualKeyboard::OpenScreenKeyboard
/// \return
///
bool VirtualKeyboard::OpenScreenKeyboard() { return OpenTabTip(); }
///
/// \brief VirtualKeyboard::OpenOSK
/// \return
///
bool VirtualKeyboard::OpenOSK() {
    PVOID oldValue = nullptr;
    // 64位系统中32位程序要访问本机system32文件夹，需取消重定向到Syswow64
    BOOL disableSuccess = Wow64DisableWow64FsRedirection(&oldValue);

    HINSTANCE result = ShellExecuteA(nullptr, "open", "osk.exe", nullptr, nullptr,
                                     SW_SHOWNORMAL);
    bool success =
        (reinterpret_cast<INT_PTR>(result) > 32); // ShellExecute 成功返回值 > 32

    if (disableSuccess) {
        Wow64RevertWow64FsRedirection(oldValue);
    }

    if (!success) {
        std::cerr << "Failed to open OSK. Error: " << GetLastError() << std::endl;
    }
    return success;
}
///
/// \brief VirtualKeyboard::OpenTabTip
/// \return
///
bool VirtualKeyboard::OpenTabTip() {
    // 如果键盘已显示，直接成功
    if (IsWin10KeyboardVisable() || IsWin7KeyboardVisable()) {
        return true;
    }

    QString tabTipPath =
        "C:\\Program Files\\Common Files\\Microsoft Shared\\ink\\TabTip.exe";
    if (!QFile::exists(tabTipPath)) {
        return false;
    }

    // 判断系统版本是否 >= Win10 14393
    if (IsNewVersion()) {
        auto pids = GetProcessIDs("TabTip.exe");
        // 如果进程未运行，直接启动并返回（启动后不一定立即显示，但调用成功）
        if (pids.isEmpty()) {
            return WShellExecute::syncExecute(tabTipPath);
        }

        // 再次检查可见性（可能启动后已显示）
        if (IsWin10KeyboardVisable() || IsWin7KeyboardVisable()) {
            return true;
        }

        // 使用 COM 接口强制显示键盘
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr)) {
            std::cerr << "CoInitialize failed: 0x" << std::hex << hr << std::endl;
            return false;
        }

        com_ptr<ITipInvocation> tip;
        hr = CoCreateInstance(CLSID_UIHostNoLaunch, nullptr,
                              CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER,
                              IID_ITipInvocation, reinterpret_cast<void **>(&tip));
        if (SUCCEEDED(hr) && tip) {
            tip->Toggle(GetDesktopWindow());
        } else {
            std::cerr << "CoCreateInstance ITipInvocation failed: 0x" << std::hex
                      << hr << std::endl;
            CoUninitialize();
            return false;
        }

        CoUninitialize();
        return true;
    } else {
        // 低版本系统直接启动进程（如果已运行，会激活窗口）
        return WShellExecute::syncExecute(tabTipPath);
    }
}
///
/// \brief VirtualKeyboard::IsWin10KeyboardVisable
/// \return
///
bool VirtualKeyboard::IsWin10KeyboardVisable() {
    HWND parent = FindWindowExA(nullptr, nullptr, WindowParentClass, nullptr);
    if (!parent)
        return false;
    HWND wnd = FindWindowExA(parent, nullptr, WindowClass, WindowCaption);
    return (wnd != nullptr);
}
///
/// \brief VirtualKeyboard::IsWin7KeyboardVisable
/// \return
///
bool VirtualKeyboard::IsWin7KeyboardVisable() {
    HWND touchWnd = FindWindowA(KeyboardWindowClass, nullptr);
    if (!touchWnd)
        return false;

    DWORD style = GetWindowLong(touchWnd, GWL_STYLE);
    // 根据经验：显示时样式包含 WS_CLIPSIBLINGS、WS_VISIBLE、WS_POPUP，且不包含
    // WS_DISABLED
    return (style & WS_CLIPSIBLINGS) && (style & WS_VISIBLE) &&
           (style & WS_POPUP) && !(style & WS_DISABLED);
}
///
/// \brief VirtualKeyboard::IsNewVersion
/// \return
///
bool VirtualKeyboard::IsNewVersion() {
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (!hMod)
        return false;

    auto RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
    if (!RtlGetVersion)
        return false;

    RTL_OSVERSIONINFOW osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (RtlGetVersion(&osvi) != 0)
        return false;

    return osvi.dwMajorVersion > 10 ||
           (osvi.dwMajorVersion == 10 && osvi.dwBuildNumber >= 14393);
}
