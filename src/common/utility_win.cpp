/*
 * Copyright (C) by Daniel Molkentin <danimo@owncloud.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "utility_win.h"
#include "utility.h"

#include <comdef.h>
#include <qt_windows.h>
#include <shlguid.h>
#include <shlobj.h>
#include <knownfolders.h>
#include <Shlwapi.h>
#include <atlbase.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QSettings>

#pragma comment(lib, "ole32.lib")

extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;

Q_LOGGING_CATEGORY(lcUtilityWin, "gui.utility.win", QtDebugMsg)

namespace {
const QString systemRunPathC() {
    return QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
}

const QString runPathC() {
    return QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
}

const QString systemThemesC()
{
    return QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize");
}

HRESULT GetShellLibraryItem(LPWSTR pwszLibraryName, IShellItem2** ppShellItem)
{
    *ppShellItem = nullptr;

    // Create the real library file name
    WCHAR wszRealLibraryName[MAX_PATH] = {0};
    swprintf_s(wszRealLibraryName, L"%s%s", pwszLibraryName, L".library-ms");

    return SHCreateItemInKnownFolder(FOLDERID_UsersLibraries, KF_FLAG_DEFAULT_PATH | KF_FLAG_NO_ALIAS, wszRealLibraryName, IID_PPV_ARGS(ppShellItem));
}

HRESULT OpenShellLibrary(LPWSTR pwszLibraryName, IShellLibrary** ppShellLib)
{
    *ppShellLib = nullptr;

    CComPtr<IShellItem2> pShellItem;
    HRESULT hr = GetShellLibraryItem(pwszLibraryName, &pShellItem);
    if (FAILED(hr)) {
        qCWarning(lcUtilityWin) << "GetShellLibraryItem" << std::system_category().message(hr);
        return hr;
    }

    // Get the shell library object from the shell item with a read and write permissions
    hr = SHLoadLibraryFromItem(pShellItem, STGM_READWRITE, IID_PPV_ARGS(ppShellLib));
    if (FAILED(hr)) {
        qCWarning(lcUtilityWin) << "SHLoadLibraryFromItem" << std::system_category().message(hr);
    }

    return hr;
}

void AddRemoveLib(bool add, const QString& folderPath, bool createLib)
{
    const auto normalizedPath = QDir::toNativeSeparators(folderPath);
    QDir d(normalizedPath);
    if (add && !d.exists()) {
        qCWarning(lcUtilityWin) << "Can't add non-existing directory" << normalizedPath;
    }

    HRESULT hr = ::CoInitialize(nullptr);
    if (FAILED(hr)) {
        qCWarning(lcUtilityWin) << "CoInitialize error: " << std::system_category().message(hr);
        return;
    }

    CComPtr<IShellLibrary> pLibrary;
    hr = OpenShellLibrary(const_cast<LPWSTR>(L"PersonalCloud Files"), &pLibrary);
    if (FAILED(hr))
    {
        qCWarning(lcUtilityWin) << "OpenShellLibrary error:" << std::system_category().message(hr);

        if (!createLib) {
            qCWarning(lcUtilityWin) << "Create lib is not set, function canceled";
            return;
        }

        hr = SHCreateLibrary(IID_PPV_ARGS(&pLibrary));
        if (FAILED(hr)) {
            qCWarning(lcUtilityWin) << "SHCreateLibrary error: " << std::system_category().message(hr);
            return;
        }

        // Save the new library under the user's Libraries folder.
        CComPtr<IShellItem> pSavedTo;
        hr = pLibrary->SaveInKnownFolder(FOLDERID_UsersLibraries, L"PersonalCloud Files", LSF_OVERRIDEEXISTING, &pSavedTo);
        if (FAILED(hr)) {
            qCWarning(lcUtilityWin) << "SaveInKnownFolder error:" << std::system_category().message(hr);
            return;
        }
    }

    if (add) {
        hr = SHAddFolderPathToLibrary(pLibrary, normalizedPath.toStdWString().c_str());
        if (FAILED(hr)) {
            qCWarning(lcUtilityWin) << "SHAddFolderPathToLibrary error:" << std::system_category().message(hr);
        }
    }
    else {
        SHRemoveFolderPathFromLibrary(pLibrary, normalizedPath.toStdWString().c_str());
        if (FAILED(hr)) {
            qCWarning(lcUtilityWin) << "SHRemoveFolderPathFromLibrary error:" << std::system_category().message(hr);
        }
    }

    if (SUCCEEDED(pLibrary->SetFolderType(FOLDERTYPEID_Documents)))
    {
        hr = pLibrary->Commit();
        if (FAILED(hr)) {
            qCWarning(lcUtilityWin) << "Library commit error:" << std::system_category().message(hr);
        }
    }

    ::CoUninitialize();
}


}

namespace APP {

void Utility::setupFavLink(const QString& localDir)
{
    qCInfo(lcUtilityWin) << "Creating Library item" << localDir;
    AddRemoveLib(true, localDir, true);
}

void Utility::removeFavLink(const QString& /*localDir*/)
{
}

bool Utility::hasSystemLaunchOnStartup(const QString &appName)
{
    QSettings settings(systemRunPathC(), QSettings::NativeFormat);
    return settings.contains(appName);
}

bool Utility::hasLaunchOnStartup(const QString &appName)
{
    QSettings settings(runPathC(), QSettings::NativeFormat);
    return settings.contains(appName);
}

void Utility::setLaunchOnStartup(const QString &appName, const QString &guiName, bool enable)
{
    Q_UNUSED(guiName)
    QSettings settings(runPathC(), QSettings::NativeFormat);
    if (enable) {
        settings.setValue(appName, QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    } else {
        settings.remove(appName);
    }
}

bool Utility::hasDarkSystray()
{
    const QSettings settings(systemThemesC(), QSettings::NativeFormat);
    return !settings.value(QStringLiteral("SystemUsesLightTheme"), false).toBool();
}

void Utility::UnixTimeToFiletime(time_t t, FILETIME *filetime)
{
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    filetime->dwLowDateTime = (DWORD) ll;
    filetime->dwHighDateTime = ll >>32;
}

void Utility::FiletimeToLargeIntegerFiletime(FILETIME *filetime, LARGE_INTEGER *hundredNSecs)
{
    hundredNSecs->LowPart = filetime->dwLowDateTime;
    hundredNSecs->HighPart = filetime->dwHighDateTime;
}

void Utility::UnixTimeToLargeIntegerFiletime(time_t t, LARGE_INTEGER *hundredNSecs)
{
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    hundredNSecs->LowPart = (DWORD) ll;
    hundredNSecs->HighPart = ll >>32;
}


QString Utility::formatWinError(long errorCode)
{
    return QStringLiteral("WindowsError: %1: %2").arg(QString::number(errorCode, 16), QString::fromWCharArray(_com_error(errorCode).ErrorMessage()));
}


Utility::NtfsPermissionLookupRAII::NtfsPermissionLookupRAII()
{
    qt_ntfs_permission_lookup++;
}

Utility::NtfsPermissionLookupRAII::~NtfsPermissionLookupRAII()
{
    qt_ntfs_permission_lookup--;
}


Utility::Handle::Handle(HANDLE h, std::function<void(HANDLE)> &&close)
    : _handle(h)
    , _close(std::move(close))
{
}

Utility::Handle::Handle(HANDLE h)
    : _handle(h)
    , _close(&CloseHandle)
{
}

Utility::Handle::~Handle()
{
    close();
}

void Utility::Handle::close()
{
    if (_handle != INVALID_HANDLE_VALUE) {
        _close(_handle);
        _handle = INVALID_HANDLE_VALUE;
    }
}


} // namespace APP
