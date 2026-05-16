/**
 * @file virtualkeyboard.cpp
 * @brief Virtual keyboard implementation using OSK and TabTip.
 * @author howdy213
 * @date 2026-05-04
 * @version 2.0.0
 *
 * @copyright Copyright (C) 2025-2026 howdy213
 *
 * This file is part of QuantifyPlugin.
 *
 * QuantifyPlugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuantifyPlugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
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

#include "WECore/file/wshellexecute.h"
#include "virtualkeyboard.h"

using namespace we;

struct ComDeleter {
    void operator()(IUnknown *ptr) const {
        if (ptr)
            ptr->Release();
    }
};

template <typename T> using com_ptr = std::unique_ptr<T, ComDeleter>;

/**
 * @brief Retrieves all process IDs of a given executable name.
 * @param fileName The name of the executable (e.g., "TabTip.exe").
 * @return List of process IDs.
 */
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

/**
 * @brief Opens the screen keyboard (default method).
 * @return True if successful.
 */
bool VirtualKeyboard::OpenScreenKeyboard() {
    return OpenTabTip();
}

/**
 * @brief Opens the traditional OSK (On-Screen Keyboard).
 * @return True if successful.
 */
bool VirtualKeyboard::OpenOSK() {
    PVOID oldValue = nullptr;
    BOOL disableSuccess = Wow64DisableWow64FsRedirection(&oldValue);

    HINSTANCE result = ShellExecuteA(nullptr, "open", "osk.exe", nullptr, nullptr,
                                     SW_SHOWNORMAL);
    bool success =
        (reinterpret_cast<INT_PTR>(result) > 32); // ShellExecute success returns > 32

    if (disableSuccess) {
        Wow64RevertWow64FsRedirection(oldValue);
    }

    if (!success) {
        qDebug() << "Failed to open OSK. Error: " << GetLastError() << "\n";
    }
    return success;
}

/**
 * @brief Opens the TabTip touch keyboard (Windows 8+).
 * @return True if successful.
 */
bool VirtualKeyboard::OpenTabTip() {
    QString tabTipPath =
        "C:\\Program Files\\Common Files\\Microsoft Shared\\ink\\TabTip.exe";
    if (!QFile::exists(tabTipPath)) {
        return false;
    }

    if (IsNewVersion()) {
        auto pids = GetProcessIDs("TabTip.exe");
        if (pids.isEmpty()) {
            return WShellExecute::syncExecute(tabTipPath);
        }

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
    } else
        return false;
}

/**
 * @brief Checks whether the system is a newer Windows version (build >= 14393).
 * @return True if version >= Windows 10 build 14393 (Anniversary Update).
 */
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