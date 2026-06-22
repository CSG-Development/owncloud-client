#include <windows.h>
#include <shellapi.h>
#include <shlobj_core.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct ClassRegistration {
    const wchar_t *clsid;
    const wchar_t *description;
    const wchar_t *dllName;
    bool contextMenuOptIn;
};

struct OverlayRegistration {
    const wchar_t *name;
    const wchar_t *clsid;
};

constexpr wchar_t kContextMenuHandlerName[] = L"PersonalCloudContextMenuHandler";
constexpr wchar_t kClassesRoot[] = L"Software\\Classes\\";
constexpr wchar_t kContextMenuHandlersKey[] = L"Software\\Classes\\AllFileSystemObjects\\shellex\\ContextMenuHandlers\\PersonalCloudContextMenuHandler";
constexpr wchar_t kOverlayIdentifiersKey[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers";
constexpr wchar_t kPayloadDirectoryName[] = L"InstallerTools\\ShellExtensionsPayload";
constexpr wchar_t kContextMenuDllName[] = L"CUContextMenu.dll";
constexpr wchar_t kOverlaysDllName[] = L"CUOverlays.dll";

constexpr std::array<ClassRegistration, 6> kClassRegistrations{{
    {L"{841A0AAD-AA11-4B50-84D9-7F8E727D77D7}", L"PersonalCloud context menu handler", kContextMenuDllName, true},
    {L"{0960F090-F328-48A3-B746-276B1E3C3722}", L"PersonalCloud overlay handler", kOverlaysDllName, false},
    {L"{0960F092-F328-48A3-B746-276B1E3C3722}", L"PersonalCloud overlay handler", kOverlaysDllName, false},
    {L"{0960F093-F328-48A3-B746-276B1E3C3722}", L"PersonalCloud overlay handler", kOverlaysDllName, false},
    {L"{0960F094-F328-48A3-B746-276B1E3C3722}", L"PersonalCloud overlay handler", kOverlaysDllName, false},
    {L"{0960F096-F328-48A3-B746-276B1E3C3722}", L"PersonalCloud overlay handler", kOverlaysDllName, false},
}};

constexpr std::array<OverlayRegistration, 5> kOverlayRegistrations{{
    {L"                PersonalCloudError", L"{0960F090-F328-48A3-B746-276B1E3C3722}"},
    {L"                PersonalCloudOK", L"{0960F092-F328-48A3-B746-276B1E3C3722}"},
    {L"                PersonalCloudOKShared", L"{0960F093-F328-48A3-B746-276B1E3C3722}"},
    {L"                PersonalCloudSync", L"{0960F094-F328-48A3-B746-276B1E3C3722}"},
    {L"                PersonalCloudWarning", L"{0960F096-F328-48A3-B746-276B1E3C3722}"},
}};

[[noreturn]] void Fail(const std::wstring &message)
{
    throw std::runtime_error(std::string(message.begin(), message.end()));
}

std::wstring JoinPath(const std::wstring &left, const std::wstring &right)
{
    if (left.empty()) {
        return right;
    }

    if (left.back() == L'\\' || left.back() == L'/') {
        return left + right;
    }

    return left + L"\\" + right;
}

std::wstring Win32ErrorMessage(DWORD errorCode)
{
    LPWSTR buffer = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD size = FormatMessageW(flags, nullptr, errorCode, 0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
    std::wstring message = size > 0 ? std::wstring(buffer, size) : L"Unknown error";
    if (buffer != nullptr) {
        LocalFree(buffer);
    }

    while (!message.empty() && (message.back() == L'\r' || message.back() == L'\n')) {
        message.pop_back();
    }

    return message;
}

void ThrowLastError(const std::wstring &context)
{
    const DWORD errorCode = GetLastError();
    Fail(context + L": " + Win32ErrorMessage(errorCode));
}

bool FileExists(const std::wstring &path)
{
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

void EnsureFileExists(const std::wstring &path)
{
    if (!FileExists(path)) {
        Fail(L"Required file not found: " + path);
    }
}

void EnsureDirectoryExists(const std::wstring &path)
{
    if (CreateDirectoryW(path.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS) {
        return;
    }

    ThrowLastError(L"Failed to create directory " + path);
}

void SetRegistryString(HKEY root, const std::wstring &subKey, const wchar_t *valueName, const std::wstring &value)
{
    HKEY key = nullptr;
    const LONG createResult = RegCreateKeyExW(root, subKey.c_str(), 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr);
    if (createResult != ERROR_SUCCESS) {
        Fail(L"Failed to create registry key " + subKey + L": " + Win32ErrorMessage(createResult));
    }

    const BYTE *data = reinterpret_cast<const BYTE *>(value.c_str());
    const DWORD dataSize = static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t));
    const LONG setResult = RegSetValueExW(key, valueName, 0, REG_SZ, data, dataSize);
    RegCloseKey(key);

    if (setResult != ERROR_SUCCESS) {
        Fail(L"Failed to set registry value for " + subKey + L": " + Win32ErrorMessage(setResult));
    }
}

void DeleteRegistryTree(HKEY root, const std::wstring &subKey)
{
    const LONG deleteResult = RegDeleteTreeW(root, subKey.c_str());
    if (deleteResult == ERROR_SUCCESS || deleteResult == ERROR_FILE_NOT_FOUND || deleteResult == ERROR_PATH_NOT_FOUND) {
        return;
    }

    Fail(L"Failed to delete registry key " + subKey + L": " + Win32ErrorMessage(deleteResult));
}

bool IsLockError(DWORD errorCode)
{
    return errorCode == ERROR_SHARING_VIOLATION || errorCode == ERROR_LOCK_VIOLATION || errorCode == ERROR_ACCESS_DENIED;
}

bool DeployBinary(const std::wstring &sourcePath, const std::wstring &targetPath)
{
    if (CopyFileW(sourcePath.c_str(), targetPath.c_str(), FALSE)) {
        return false;
    }

    const DWORD copyError = GetLastError();
    if (!IsLockError(copyError)) {
        ThrowLastError(L"Failed to copy " + sourcePath + L" to " + targetPath);
    }

    const std::wstring pendingPath = targetPath + L".pending";
    DeleteFileW(pendingPath.c_str());

    if (!CopyFileW(sourcePath.c_str(), pendingPath.c_str(), FALSE)) {
        ThrowLastError(L"Failed to create pending replacement for " + targetPath);
    }

    if (!MoveFileExW(pendingPath.c_str(), targetPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT)) {
        ThrowLastError(L"Failed to schedule reboot-time replacement for " + targetPath);
    }

    return true;
}

bool RemoveBinary(const std::wstring &path)
{
    if (DeleteFileW(path.c_str())) {
        return false;
    }

    const DWORD deleteError = GetLastError();
    if (deleteError == ERROR_FILE_NOT_FOUND || deleteError == ERROR_PATH_NOT_FOUND) {
        return false;
    }

    if (!IsLockError(deleteError)) {
        ThrowLastError(L"Failed to delete " + path);
    }

    if (!MoveFileExW(path.c_str(), nullptr, MOVEFILE_DELAY_UNTIL_REBOOT)) {
        ThrowLastError(L"Failed to schedule reboot-time deletion for " + path);
    }

    return true;
}

void RegisterClass(const ClassRegistration &registration, const std::wstring &dllPath)
{
    const std::wstring clsidRoot = std::wstring(kClassesRoot) + L"CLSID\\" + registration.clsid;
    const std::wstring inprocServerKey = clsidRoot + L"\\InprocServer32";

    SetRegistryString(HKEY_LOCAL_MACHINE, clsidRoot, nullptr, registration.description);
    SetRegistryString(HKEY_LOCAL_MACHINE, inprocServerKey, nullptr, dllPath);
    SetRegistryString(HKEY_LOCAL_MACHINE, inprocServerKey, L"ThreadingModel", L"apartment");

    if (registration.contextMenuOptIn) {
        SetRegistryString(HKEY_LOCAL_MACHINE, clsidRoot, L"ContextMenuOptIn", L"");
    }
}

void RegisterContextMenuHandler()
{
    SetRegistryString(HKEY_LOCAL_MACHINE, kContextMenuHandlersKey, nullptr, kClassRegistrations.front().clsid);
}

void RegisterOverlayHandlers()
{
    for (const auto &overlay : kOverlayRegistrations) {
        const std::wstring key = std::wstring(kOverlayIdentifiersKey) + L"\\" + overlay.name;
        SetRegistryString(HKEY_LOCAL_MACHINE, key, nullptr, overlay.clsid);
    }
}

void UnregisterContextMenuHandler()
{
    DeleteRegistryTree(HKEY_LOCAL_MACHINE, kContextMenuHandlersKey);
}

void UnregisterOverlayHandlers()
{
    for (const auto &overlay : kOverlayRegistrations) {
        const std::wstring key = std::wstring(kOverlayIdentifiersKey) + L"\\" + overlay.name;
        DeleteRegistryTree(HKEY_LOCAL_MACHINE, key);
    }
}

void UnregisterClasses()
{
    for (const auto &registration : kClassRegistrations) {
        const std::wstring key = std::wstring(kClassesRoot) + L"CLSID\\" + registration.clsid;
        DeleteRegistryTree(HKEY_LOCAL_MACHINE, key);
    }
}

bool ApplyShellExtensions(const std::wstring &targetDir)
{
    const std::wstring payloadDir = JoinPath(targetDir, kPayloadDirectoryName);
    const std::wstring contextMenuSource = JoinPath(payloadDir, kContextMenuDllName);
    const std::wstring overlaysSource = JoinPath(payloadDir, kOverlaysDllName);
    const std::wstring contextMenuTarget = JoinPath(targetDir, kContextMenuDllName);
    const std::wstring overlaysTarget = JoinPath(targetDir, kOverlaysDllName);

    EnsureFileExists(contextMenuSource);
    EnsureFileExists(overlaysSource);
    EnsureDirectoryExists(targetDir);

    bool rebootRequired = false;
    rebootRequired = DeployBinary(contextMenuSource, contextMenuTarget) || rebootRequired;
    rebootRequired = DeployBinary(overlaysSource, overlaysTarget) || rebootRequired;

    RegisterClass(kClassRegistrations[0], contextMenuTarget);
    for (size_t i = 1; i < kClassRegistrations.size(); ++i) {
        RegisterClass(kClassRegistrations[i], overlaysTarget);
    }

    RegisterContextMenuHandler();
    RegisterOverlayHandlers();
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    if (rebootRequired) {
        std::wcout << L"Shell extension binaries are scheduled for replacement on reboot." << std::endl;
    }

    return rebootRequired;
}

bool RemoveShellExtensions(const std::wstring &targetDir)
{
    const std::wstring contextMenuTarget = JoinPath(targetDir, kContextMenuDllName);
    const std::wstring overlaysTarget = JoinPath(targetDir, kOverlaysDllName);

    UnregisterContextMenuHandler();
    UnregisterOverlayHandlers();
    UnregisterClasses();
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    bool rebootRequired = false;
    rebootRequired = RemoveBinary(contextMenuTarget) || rebootRequired;
    rebootRequired = RemoveBinary(overlaysTarget) || rebootRequired;

    if (rebootRequired) {
        std::wcout << L"Shell extension binaries are scheduled for deletion on reboot." << std::endl;
    }

    return rebootRequired;
}

void PrintUsage()
{
    std::wcerr << L"Usage: ShellExtensionRegistrar.exe <apply|remove> <target-dir>" << std::endl;
}

} // namespace

int wmain(int argc, wchar_t *argv[])
{
    try {
        if (argc != 3) {
            PrintUsage();
            return 2;
        }

        const std::wstring command = argv[1];
        const std::wstring targetDir = argv[2];

        if (command == L"apply") {
            ApplyShellExtensions(targetDir);
            return 0;
        }

        if (command == L"remove") {
            RemoveShellExtensions(targetDir);
            return 0;
        }

        PrintUsage();
        return 2;
    }
    catch (const std::exception &error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
