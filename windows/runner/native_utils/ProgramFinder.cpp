// Define NOMINMAX *before* including Windows.h to prevent macro conflicts
#define NOMINMAX

#include <windows.h>
#include <shlwapi.h>      // Path functions, _wcsicmp, PathParseIconLocationW, SearchPathW, PathFindExtension
#include <shlobj.h>       // SHGetKnownFolderPath, FOLDERID_ constants
#include <Shellapi.h>     // CommandLineToArgvW, ExtractIconExW, PrivateExtractIconsW
#include <shlguid.h>      // CLSID_ShellLink, IID_IShellLinkW
#include <combaseapi.h>   // CoInitializeEx, CoUninitialize, CoCreateInstance, CoTaskMemFree
#include <filesystem>     // For path manipulation and iteration (C++17)
#include <memory>         // For std::unique_ptr
#include <map>            // For deduplication map
#include <set>            // For deduplication set
#include <vector>         // For storing results
#include <string>         // For std::string, std::wstring
#include <sstream>        // Needed for reconstructing args and debug output
#include <algorithm>      // For std::sort, std::transform
#include <cctype>         // For ::towlower
#include <optional>       // For std::optional return types
#include <utility>        // For std::pair, std::move
#include <iostream>       // For std::cerr
#include <wingdi.h>       // For GDI objects (related to icons)
#include <wincrypt.h>     // For CryptBinaryToStringA (Base64)
#include <gdiplus.h>      // For GDI+ icon processing (Bitmap, Graphics)
#include <objidl.h>       // For IStream
#include <system_error>   // For std::error_code
#include <limits>         // For std::numeric_limits
#include <knownfolders.h> // For KNOWNFOLDERID definitions

// Assuming ProgramFinder.h defines the Program struct like this:
#include "ProgramFinder.h"

#include "SettingsPages.h"
// #include "UwpFinder.h"

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Gdiplus.lib")

#define PFINDER_DEBUG // Enable debug output
#ifdef PFINDER_DEBUG
#include <debugapi.h>
template <typename... Args>
void DebugOutput(Args &&...args)
{
    std::wstringstream wss;
    (wss << ... << std::forward<Args>(args));
    wss << L"\n";
    OutputDebugStringW(wss.str().c_str());
}
#else
#define DebugOutput(...) \
    do                   \
    {                    \
    } while (0)
#endif

namespace ProgramFinder
{
    namespace fs = std::filesystem;
    using namespace Gdiplus;

    // ================================================================
    // >>> START Anonymous Namespace for Internal Implementation <<<
    // ================================================================
    namespace
    {
        // --- RAII Wrappers ---
        struct CoInitializer
        {
            HRESULT hr;
            CoInitializer();
            ~CoInitializer();
            bool IsInitialized() const;
        };
        CoInitializer::CoInitializer() : hr(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))
        {
            if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
            {
                std::cerr << "PFErr: COM Init failed. HRESULT=0x" << std::hex << hr << std::dec << std::endl;
                hr = E_FAIL;
            }
            else if (hr == S_FALSE || SUCCEEDED(hr))
            {
                hr = S_OK;
                DebugOutput(L"COM Initialized successfully.");
            }
        }
        CoInitializer::~CoInitializer()
        {
            if (SUCCEEDED(hr))
            {
                CoUninitialize();
                DebugOutput(L"COM Uninitialized.");
            }
        }
        bool CoInitializer::IsInitialized() const { return SUCCEEDED(hr); }

        struct GdiplusInitializer
        {
            ULONG_PTR t = 0;
            Status s = GenericError;
            GdiplusInitializer()
            {
                GdiplusStartupInput i;
                s = GdiplusStartup(&t, &i, 0);
                if (s != Ok)
                {
                    std::cerr << "PFErr: GDI+ Startup failed. Status: " << s << std::endl;
                }
                else
                {
                    DebugOutput(L"GDI+ Initialized successfully.");
                }
            }
            ~GdiplusInitializer()
            {
                if (s == Ok && t != 0)
                {
                    GdiplusShutdown(t);
                    DebugOutput(L"GDI+ Shutdown.");
                }
            }
            bool IsInitialized() const { return s == Ok; }
        };
        // --- Deleters ---
        struct ComReleaser
        {
            void operator()(IUnknown *p) const
            {
                if (p)
                    p->Release();
            }
        };
        template <typename T>
        using ComUniquePtr = std::unique_ptr<T, ComReleaser>;
        struct HIconDeleter
        {
            void operator()(HICON h) const
            {
                if (h)
                    DestroyIcon(h);
            }
        };
        using HIconUniquePtr = std::unique_ptr<HICON__, HIconDeleter>;
        struct RegKeyDeleter
        {
            void operator()(HKEY h) const
            {
                if (h && h != INVALID_HANDLE_VALUE)
                    RegCloseKey(h);
            }
        };
        using RegKeyUniquePtr = std::unique_ptr<HKEY__, RegKeyDeleter>;
        struct CoTaskMemDeleter
        {
            void operator()(void *pv) const
            {
                if (pv)
                    CoTaskMemFree(pv);
            }
        };
        template <typename T>
        using CoTaskMemUniquePtr = std::unique_ptr<T, CoTaskMemDeleter>;
        struct LocalMemDeleter
        {
            void operator()(HLOCAL h) const
            {
                if (h)
                    LocalFree(h);
            }
        };
        using LocalMemUniquePtr = std::unique_ptr<void, LocalMemDeleter>;

        // --- Registry Scanning ---
        std::vector<utils::Program> GetInstalledProgramsFromRegistryInternal(); // Definition below (unchanged)

        // --- Start Menu Scanning (Uses Fallback Flag) ---
        // --- Start Menu Scanning (Modified for Description/Kind) ---
        std::vector<utils::Program> GetProgramsFromStartMenuInternal()
        {
            std::vector<utils::Program> programs;
            const KNOWNFOLDERID knownFolderIds[] = {FOLDERID_CommonPrograms, FOLDERID_Programs};
            const char *sourceNames[] = {"Start Menu (Common)", "Start Menu (User)"};

            for (size_t i = 0; i < ARRAYSIZE(knownFolderIds); ++i)
            {
                PWSTR folderPathRaw = nullptr;
                HRESULT hr = SHGetKnownFolderPath(knownFolderIds[i], KF_FLAG_DEFAULT, NULL, &folderPathRaw);
                CoTaskMemUniquePtr<WCHAR> folderPathPtr(folderPathRaw);
                if (SUCCEEDED(hr) && folderPathPtr && folderPathPtr.get()[0] != L'\0')
                {
                    fs::path startMenuPath;
                    try
                    {
                        startMenuPath = fs::path(folderPathPtr.get());
                    }
                    catch (...)
                    {
                        continue;
                    }
                    std::error_code ec;
                    if (!fs::exists(startMenuPath, ec) || ec || !fs::is_directory(startMenuPath, ec) || ec)
                    {
                        continue;
                    }

                    DebugOutput(L"SM: Scanning Start Menu folder: '", startMenuPath.wstring().c_str(), L"'");
                    try
                    {
                        fs::recursive_directory_iterator dir_iter(startMenuPath, fs::directory_options::skip_permission_denied, ec);
                        if (ec)
                        {
                            DebugOutput(L"SM Iter ERR: Failed iterator create: ", utils::Utf8ToWide(ec.message()).c_str());
                            continue;
                        }
                        fs::recursive_directory_iterator end_iter;

                        while (dir_iter != end_iter)
                        {
                            try
                            {
                                const auto &entry = *dir_iter;
                                fs::path entryPathFs = entry.path();
                                std::error_code file_ec;
                                if (entry.is_regular_file(file_ec) && !file_ec && entryPathFs.has_extension() && _wcsicmp(entryPathFs.extension().c_str(), L".lnk") == 0)
                                {
                                    // *** Assume utils::ResolveShortcut populates descriptionUtf8 and potentially iconDataBase64 ***
                                    std::optional<utils::ShortcutInfo> shortcutInfoOpt = utils::ResolveShortcut(entryPathFs);

                                    if (shortcutInfoOpt)
                                    {
                                        const utils::ShortcutInfo &shortcutInfo = *shortcutInfoOpt;
                                        utils::Program p;
                                        p.name = utils::WideToUtf8(entryPathFs.stem().wstring());
                                        if (p.name.empty())
                                        {
                                            p.name = "Unnamed Shortcut Program";
                                        }

                                        p.arguments = shortcutInfo.argumentsUtf8;
                                        p.iconPath = shortcutInfo.iconPathUtf8; // Assume already normalized by ResolveShortcut
                                        p.iconIndex = shortcutInfo.iconIndex;
                                        p.source = sourceNames[i];
                                        p.kind = "link";                              // <-- Assign Kind for shortcuts
                                        p.description = shortcutInfo.descriptionUtf8; // <-- Assign Description from shortcut

                                        // Adjust executable path based on fallback flag
                                        if (shortcutInfo.isFallbackPath)
                                        {
                                            p.executablePath = utils::NormalizePath(utils::WideToUtf8(entryPathFs.wstring())); // Use LNK path
                                            DebugOutput(L"SM: Using LNK path as executable for '", utils::Utf8ToWide(p.name).c_str(), L"' because resolution used fallback path ('", utils::Utf8ToWide(shortcutInfo.resolvedTargetPathUtf8).c_str(), L"').");
                                        }
                                        else
                                        {
                                            p.executablePath = shortcutInfo.resolvedTargetPathUtf8; // Use resolved target (Assume already normalized)
                                        }

                                        // Final validation
                                        if (p.executablePath.empty())
                                        {
                                            DebugOutput(L"SM ERR: Skipping '", utils::Utf8ToWide(p.name).c_str(), L"' - Final executable path empty.");
                                            // Safe increment handled below, just continue loop iteration
                                        }
                                        else
                                        {
                                            // Fallback Icon Path if needed (iconPath wasn't set or ResolveShortcut failed normalization)
                                            if (p.iconPath.empty())
                                            {
                                                p.iconPath = p.executablePath; // Fallback to executable
                                                p.iconIndex = 0;
                                            }
                                            // Ensure non-negative index if path exists
                                            if (p.iconIndex < 0 && !p.iconPath.empty())
                                                p.iconIndex = 0;

                                            // Get icon data: Use pre-extracted if available, otherwise extract now
                                            if (!shortcutInfo.iconDataBase64.empty())
                                            {
                                                p.iconDataBase64 = shortcutInfo.iconDataBase64; // Copy if ResolveShortcut provided it
                                                DebugOutput(L"SM: Using pre-extracted icon data for '", utils::Utf8ToWide(p.name).c_str(), L"'");
                                            }
                                            // Only try extracting if we have a valid path/index AND ResolveShortcut didn't provide data
                                            else if (!p.iconPath.empty() && p.iconIndex >= 0)
                                            {
                                                // Call the extraction function defined within the anonymous namespace
                                                auto encodedIconOpt = utils::ExtractAndEncodeIconAsBase64(p.iconPath, p.iconIndex);
                                                if (encodedIconOpt)
                                                {
                                                    p.iconDataBase64 = std::move(*encodedIconOpt); // Move the data
                                                }
                                                else
                                                {
                                                    DebugOutput(L"SM: Icon extraction failed for '", utils::Utf8ToWide(p.name), L"' (Path: '", utils::Utf8ToWide(p.iconPath), L"', Index: ", p.iconIndex, L")");
                                                }
                                            }

                                            DebugOutput(L"SM: Adding Program: Name='", utils::Utf8ToWide(p.name).c_str(),
                                                        L"', FinalExecPath='", utils::Utf8ToWide(p.executablePath).c_str(),
                                                        L"', Kind='", utils::Utf8ToWide(p.kind).c_str(),
                                                        L"', Desc='", utils::Utf8ToWide(p.description).c_str(),
                                                        L"', IconPath='", utils::Utf8ToWide(p.iconPath).c_str(), L"', IconIndex=", p.iconIndex);
                                            programs.push_back(std::move(p));
                                        }
                                    }
                                    else
                                    {
                                        DebugOutput(L"SM INFO: Skipping LNK ('", entryPathFs.wstring().c_str(), L"') - Resolve failed.");
                                    }
                                } // End if (is .lnk file)
                            } // End inner try
                            catch (const fs::filesystem_error &fe)
                            {
                                DebugOutput(L"SM Entry Err FS: ", utils::Utf8ToWide(fe.what()).c_str());
                                (void)fe;
                            }
                            catch (const std::exception &se)
                            {
                                DebugOutput(L"SM Entry Err STD: ", utils::Utf8ToWide(se.what()).c_str());
                                (void)se;
                            }
                            catch (...)
                            {
                                DebugOutput(L"SM Entry Err UNK");
                            }

                            // Safely increment iterator
                            try
                            {
                                dir_iter.increment(ec);
                                if (ec)
                                {
                                    try
                                    {
                                        dir_iter.pop();
                                    }
                                    catch (...)
                                    {
                                        break;
                                    }
                                    ec.clear();
                                }
                            }
                            catch (...)
                            {
                                DebugOutput(L"SM Iter Incr Exception. Breaking loop.");
                                break;
                            }
                        } // End while
                    }
                    catch (const std::exception &e)
                    {
                        DebugOutput(L"SM Iter Setup Err: ", utils::Utf8ToWide(e.what()).c_str());
                    }
                    catch (...)
                    {
                        DebugOutput(L"SM Iter Setup Err UNK");
                    }
                }
                else
                {
                    DebugOutput(L"SM ERR: SHGetKnownFolderPath failed H=0x", std::hex, hr);
                }
            } // End loop (known folders)
            return programs;
        }

        // --- Registry Scanning (Modified for Description/Kind) ---
        std::vector<utils::Program> GetInstalledProgramsFromRegistryInternal()
        {
            std::vector<utils::Program> programs;
            const HKEY hiveRoots[] = {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER};
            const WCHAR *uninstallKeysW[] = {
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
                L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"};

            for (HKEY hRootKey : hiveRoots)
            {
                std::string rootKeyName = (hRootKey == HKEY_LOCAL_MACHINE) ? "HKLM" : "HKCU";
                for (const WCHAR *regPath : uninstallKeysW)
                {
                    REGSAM wow64Access = KEY_WOW64_64KEY;
                    if (wcsstr(regPath, L"WOW6432Node") != nullptr)
                        wow64Access = KEY_WOW64_32KEY;

                    HKEY hUninstallKeyRaw = NULL;
                    LSTATUS openStatus = RegOpenKeyExW(hRootKey, regPath, 0, KEY_READ | KEY_ENUMERATE_SUB_KEYS | wow64Access, &hUninstallKeyRaw);
                    if (openStatus != ERROR_SUCCESS)
                    {
                        if (openStatus != ERROR_FILE_NOT_FOUND)
                        { // Don't log expected "not found" errors
                            DebugOutput(L"REG WARN: Failed to open key '", regPath, L"' in ", utils::Utf8ToWide(rootKeyName).c_str(), L". Status: ", openStatus);
                        }
                        continue;
                    }
                    RegKeyUniquePtr hUninstallKey(hUninstallKeyRaw);

                    DWORD subKeyIndex = 0;
                    WCHAR subKeyNameW[256];
                    while (true)
                    {
                        DWORD subKeyNameLen = ARRAYSIZE(subKeyNameW);
                        LSTATUS enumStatus = RegEnumKeyExW(hUninstallKey.get(), subKeyIndex++, subKeyNameW, &subKeyNameLen, NULL, NULL, NULL, NULL);
                        if (enumStatus == ERROR_NO_MORE_ITEMS)
                            break;
                        if (enumStatus != ERROR_SUCCESS)
                            continue;

                        HKEY hAppKeyRaw = NULL;
                        LSTATUS appKeyStatus = RegOpenKeyExW(hUninstallKey.get(), subKeyNameW, 0, KEY_READ | wow64Access, &hAppKeyRaw);
                        if (appKeyStatus == ERROR_SUCCESS)
                        {
                            RegKeyUniquePtr hAppKey(hAppKeyRaw);
                            DWORD valueType = 0;
                            std::string displayNameUtf8;
                            std::string descriptionUtf8;   // For description
                            std::wstring executablePathW;  // Parsed executable path
                            std::wstring argumentsW;       // Parsed arguments
                            std::wstring iconPathW;        // Parsed icon path
                            int iconIndex = 0;             // Parsed icon index
                            std::wstring uninstallStringW; // Raw uninstall string

                            // --- Get DisplayName ---
                            WCHAR displayNameW[512] = {0};
                            DWORD displayNameSize = sizeof(displayNameW);
                            LSTATUS nameStatus = RegQueryValueExW(hAppKey.get(), L"DisplayName", 0, &valueType, reinterpret_cast<LPBYTE>(displayNameW), &displayNameSize);
                            if (nameStatus == ERROR_SUCCESS && valueType == REG_SZ && displayNameW[0] != L'\0')
                            {
                                displayNameUtf8 = utils::WideToUtf8(displayNameW);
                            }
                            else
                            {
                                displayNameUtf8 = utils::WideToUtf8(subKeyNameW); // Fallback to key name
                                wcscpy_s(displayNameW, ARRAYSIZE(displayNameW), subKeyNameW);
                            }

                            // --- Check Filters (SystemComponent, Updates etc.) ---
                            DWORD systemComponent = 0;
                            DWORD systemComponentSize = sizeof(systemComponent);
                            bool isSystemComponent = (RegQueryValueExW(hAppKey.get(), L"SystemComponent", 0, &valueType, reinterpret_cast<LPBYTE>(&systemComponent), &systemComponentSize) == ERROR_SUCCESS && valueType == REG_DWORD && systemComponent == 1);

                            bool isWindowsInstaller = false;
                            DWORD winInstallerFlag = 0;
                            DWORD winInstallerFlagSize = sizeof(winInstallerFlag);
                            if (RegQueryValueExW(hAppKey.get(), L"WindowsInstaller", 0, &valueType, reinterpret_cast<LPBYTE>(&winInstallerFlag), &winInstallerFlagSize) == ERROR_SUCCESS && valueType == REG_DWORD && winInstallerFlag == 1)
                            {
                                isWindowsInstaller = true;
                            }
                            bool isUpdateEtc = (wcsstr(displayNameW, L"KB") == displayNameW || wcsstr(displayNameW, L"Security Update") != nullptr || wcsstr(displayNameW, L"Update for Microsoft") != nullptr);

                            if (displayNameUtf8.empty() || isSystemComponent || isUpdateEtc)
                            {
                                DebugOutput(L"REG: Skipping '", displayNameW, L"' (System/Update/Empty)");
                                continue;
                            }

                            // --- Get UninstallString (or QuietUninstallString) ---
                            WCHAR uninstallBufW[MAX_PATH * 2] = {0};
                            DWORD uninstallBufSize = sizeof(uninstallBufW);
                            bool gotUninstall = false;
                            if (RegQueryValueExW(hAppKey.get(), L"QuietUninstallString", 0, &valueType, reinterpret_cast<LPBYTE>(uninstallBufW), &uninstallBufSize) == ERROR_SUCCESS && valueType == REG_SZ && uninstallBufW[0] != L'\0')
                            {
                                uninstallStringW = uninstallBufW;
                                gotUninstall = true;
                            }
                            if (!gotUninstall)
                            {
                                uninstallBufSize = sizeof(uninstallBufW);
                                if (RegQueryValueExW(hAppKey.get(), L"UninstallString", 0, &valueType, reinterpret_cast<LPBYTE>(uninstallBufW), &uninstallBufSize) == ERROR_SUCCESS && valueType == REG_SZ && uninstallBufW[0] != L'\0')
                                {
                                    uninstallStringW = uninstallBufW;
                                    gotUninstall = true;
                                }
                            }

                            if ((!gotUninstall || uninstallStringW.empty()) && isWindowsInstaller)
                            {
                                DebugOutput(L"REG: Skipping '", displayNameW, L"' (WindowsInstaller entry without explicit UninstallString)");
                                continue;
                            }
                            if (!gotUninstall || uninstallStringW.empty())
                            {
                                DebugOutput(L"REG: Skipping '", displayNameW, L"' (No UninstallString found)");
                                continue;
                            }

                            // --- NEW: Parse Executable and Arguments from UninstallString ---
                            WCHAR potentialPathW[MAX_PATH * 2] = {0};
                            bool isQuoted = (!uninstallStringW.empty() && uninstallStringW.front() == L'"');

                            if (isQuoted)
                            {
                                // Path is quoted, use CommandLineToArgvW
                                int argc = 0;
                                LocalMemUniquePtr argvW_ptr((LPWSTR *)CommandLineToArgvW(uninstallStringW.c_str(), &argc));
                                if (argvW_ptr && argc > 0)
                                {
                                    LPWSTR *argvW = (LPWSTR *)argvW_ptr.get();
                                    std::wstring firstArg = argvW[0];
                                    // PathUnquoteSpacesW(&firstArg[0]); // CommandLineToArgvW handles quotes, PathUnquoteSpaces might remove needed internal quotes if any. SearchPathW should handle it.

                                    // Resolve path using SearchPathW
                                    DWORD searchResult = SearchPathW(NULL, firstArg.c_str(), L".exe", ARRAYSIZE(potentialPathW), potentialPathW, NULL);
                                    if (searchResult > 0 && searchResult < ARRAYSIZE(potentialPathW))
                                    {
                                        executablePathW = potentialPathW; // Resolved path
                                    }
                                    else
                                    {
                                        // Fallback: Use the first argument as is, if it exists (less reliable for quoted)
                                        executablePathW = firstArg; // Use unquoted first arg
                                        DebugOutput(L"REG WARN: SearchPathW failed for quoted arg '", firstArg.c_str(), L"' in '", displayNameW, L"'. Using arg as path.");
                                    }

                                    // Reconstruct arguments
                                    std::wstringstream ssArgs;
                                    for (int k = 1; k < argc; ++k)
                                    {
                                        std::wstring arg = argvW[k];
                                        bool needsQuotes = (arg.find(L' ') != std::wstring::npos || arg.find(L'\t') != std::wstring::npos) && (arg.length() > 0 && arg.front() != L'"');
                                        if (k > 1) ssArgs << L" ";
                                        if (needsQuotes) ssArgs << L"\"";
                                        ssArgs << arg;
                                        if (needsQuotes) ssArgs << L"\"";
                                    }
                                    argumentsW = ssArgs.str();
                                }
                                else
                                {
                                    // Parsing failed, fallback to treating the whole (unquoted) string as path
                                    std::wstring unquotedUninstall = uninstallStringW;
                                    PathUnquoteSpacesW(&unquotedUninstall[0]); // Unquote the original string
                                    executablePathW = unquotedUninstall;
                                    argumentsW = L"";
                                    DebugOutput(L"REG INFO: CommandLineToArgvW failed for quoted '", displayNameW, L"', using unquoted string as path.");
                                }
                            }
                            else // Not quoted
                            {
                                // Try resolving the entire unquoted string first
                                DWORD searchResult = SearchPathW(NULL, uninstallStringW.c_str(), L".exe", ARRAYSIZE(potentialPathW), potentialPathW, NULL);
                                if (searchResult > 0 && searchResult < ARRAYSIZE(potentialPathW) && utils::DoesPathExist(potentialPathW))
                                {
                                    // Success: Entire string resolves to a valid executable path
                                    executablePathW = potentialPathW;
                                    argumentsW = L"";
                                    DebugOutput(L"REG INFO: Unquoted UninstallString '", uninstallStringW.c_str(), L"' resolved directly for '", displayNameW, L"'. Assuming no args.");
                                }
                                else
                                {
                                    // Whole string didn't resolve, now try CommandLineToArgvW
                                    int argc = 0;
                                    LocalMemUniquePtr argvW_ptr((LPWSTR *)CommandLineToArgvW(uninstallStringW.c_str(), &argc));
                                    if (argvW_ptr && argc > 0)
                                    {
                                        LPWSTR *argvW = (LPWSTR *)argvW_ptr.get();
                                        std::wstring firstArg = argvW[0];
                                        // PathUnquoteSpacesW(&firstArg[0]); // Not needed as it wasn't quoted initially

                                        // Resolve first argument using SearchPathW
                                        searchResult = SearchPathW(NULL, firstArg.c_str(), L".exe", ARRAYSIZE(potentialPathW), potentialPathW, NULL);
                                        if (searchResult > 0 && searchResult < ARRAYSIZE(potentialPathW))
                                        {
                                            executablePathW = potentialPathW; // Resolved path from first arg
                                        }
                                        else
                                        {
                                            // Fallback: Use the first argument as is
                                            executablePathW = firstArg;
                                            DebugOutput(L"REG WARN: SearchPathW failed for first arg '", firstArg.c_str(), L"' of unquoted string for '", displayNameW, L"'. Using arg as path.");
                                        }

                                        // Reconstruct arguments from the rest
                                        std::wstringstream ssArgs;
                                        for (int k = 1; k < argc; ++k)
                                        {
                                            std::wstring arg = argvW[k];
                                            bool needsQuotes = (arg.find(L' ') != std::wstring::npos || arg.find(L'\t') != std::wstring::npos) && (arg.length() > 0 && arg.front() != L'"');
                                            if (k > 1) ssArgs << L" ";
                                            if (needsQuotes) ssArgs << L"\"";
                                            ssArgs << arg;
                                            if (needsQuotes) ssArgs << L"\"";
                                        }
                                        argumentsW = ssArgs.str();
                                    }
                                    else
                                    {
                                        // Parsing failed completely, fallback to treating the whole string as path
                                        executablePathW = uninstallStringW;
                                        argumentsW = L"";
                                        DebugOutput(L"REG INFO: CommandLineToArgvW failed for unquoted '", displayNameW, L"', using original string as path.");
                                    }
                                }
                            }
                            // --- END NEW PARSING LOGIC ---


                            if (executablePathW.empty())
                            {
                                DebugOutput(L"REG: Skipping '", displayNameW, L"' (Could not determine executable path)");
                                continue;
                            }

                            // --- Get DisplayIcon ---
                            WCHAR displayIconW[MAX_PATH * 2] = {0};
                            DWORD displayIconSize = sizeof(displayIconW);
                            iconIndex = 0; // Default index

                            if (RegQueryValueExW(hAppKey.get(), L"DisplayIcon", 0, &valueType, reinterpret_cast<LPBYTE>(displayIconW), &displayIconSize) == ERROR_SUCCESS && valueType == REG_SZ && displayIconW[0] != L'\0')
                            {
                                // Expand environment variables (e.g., %SystemRoot%)
                                WCHAR expandedIconPathW[MAX_PATH * 2] = {0};
                                DWORD expandedLen = ExpandEnvironmentStringsW(displayIconW, expandedIconPathW, ARRAYSIZE(expandedIconPathW));

                                // Use expanded path if successful, otherwise fallback to raw path
                                WCHAR *pathBuffer = (expandedLen > 0 && expandedLen < ARRAYSIZE(expandedIconPathW)) ? expandedIconPathW : displayIconW;

                                // --- CORRECT USAGE OF PathParseIconLocationW ---
                                // 1. Call PathParseIconLocationW - it RETURNS the index and MODIFIES the buffer
                                iconIndex = PathParseIconLocationW(pathBuffer); // This returns the index and modifies pathBuffer
                                // 2. pathBuffer now holds ONLY the path part
                                iconPathW = pathBuffer; // Assign the modified buffer to iconPathW
                                // --- END CORRECT USAGE ---

                                // Check if the index is valid, default to 0 if negative
                                if (iconIndex < 0)
                                {
                                    DebugOutput(L"REG WARN: Negative icon index returned for '", displayNameW, L"'. Defaulting to 0.");
                                    iconIndex = 0;
                                }

                                // Verify the path exists after parsing
                                if (!utils::DoesPathExist(iconPathW))
                                {
                                    DebugOutput(L"REG WARN: Parsed icon path '", iconPathW.c_str(), L"' does not exist for '", displayNameW, L"'. Will fallback.");
                                    iconPathW.clear(); // Clear path so fallback is triggered
                                }
                            }
                            // If no valid DisplayIcon found or path didn't exist after parsing, iconPathW remains empty

                            // --- Get Comments (Description) ---
                            WCHAR commentsW[1024] = {0}; // Buffer for comments
                            DWORD commentsSize = sizeof(commentsW);
                            if (RegQueryValueExW(hAppKey.get(), L"Comments", 0, &valueType, reinterpret_cast<LPBYTE>(commentsW), &commentsSize) == ERROR_SUCCESS && valueType == REG_SZ && commentsW[0] != L'\0')
                            {
                                descriptionUtf8 = utils::WideToUtf8(commentsW);
                            }

                            // --- Create and Populate Program Struct ---
                            utils::Program prog;
                            prog.name = displayNameUtf8;
                            prog.source = rootKeyName + " Uninstall";
                            prog.executablePath = utils::NormalizePath(utils::WideToUtf8(executablePathW));
                            prog.kind = "program";              // Assign Kind for registry entries
                            prog.description = descriptionUtf8; // Assign Description

                            if (prog.executablePath.empty() && !executablePathW.empty())
                            {
                                DebugOutput(L"REG WARN: Normalization/UTF8 failed for executable path '", executablePathW.c_str(), L"'. Skipping entry '", displayNameW, L"'.");
                                continue;
                            }
                            if (prog.executablePath.empty())
                            {
                                DebugOutput(L"REG WARN: Executable path became empty for '", displayNameW, L"'. Skipping.");
                                continue;
                            }

                            prog.arguments = utils::WideToUtf8(argumentsW);
                            if (prog.arguments.empty() && !argumentsW.empty())
                            {
                                DebugOutput(L"REG WARN: UTF8 conversion failed for arguments '", argumentsW.c_str(), L"'. Args cleared.");
                                prog.arguments.clear();
                            }

                            // --- Finalize icon path ---
                            std::string finalIconPathUtf8;
                            int finalIconIndex = 0;

                            // Use parsed iconPathW if it exists and is valid
                            if (!iconPathW.empty())
                            {
                                finalIconPathUtf8 = utils::NormalizePath(utils::WideToUtf8(iconPathW));
                                if (!finalIconPathUtf8.empty())
                                {
                                    finalIconIndex = iconIndex; // Use the index parsed from DisplayIcon
                                }
                                else
                                {
                                    DebugOutput(L"REG WARN: Normalization/UTF8 failed for icon path '", iconPathW, L"'. Will fallback.");
                                }
                            }

                            // If icon path is still empty (no DisplayIcon, parse failed, normalization failed, or path didn't exist),
                            // use the main executable path as the icon source.
                            if (finalIconPathUtf8.empty())
                            {
                                finalIconPathUtf8 = prog.executablePath; // Already normalized
                                finalIconIndex = 0;                      // Default index for executable
                            }

                            // Ensure index isn't negative (redundant check, but safe)
                            if (finalIconIndex < 0)
                                finalIconIndex = 0;

                            prog.iconPath = finalIconPathUtf8;
                            prog.iconIndex = finalIconIndex;

                            // Extract and encode icon
                            if (!prog.iconPath.empty() && prog.iconIndex >= 0)
                            {
                                // Call the extraction function defined within the anonymous namespace
                                auto encodedIconOpt = utils::ExtractAndEncodeIconAsBase64(prog.iconPath, prog.iconIndex);
                                if (encodedIconOpt)
                                {
                                    prog.iconDataBase64 = std::move(*encodedIconOpt);
                                }
                                else
                                {
                                    DebugOutput(L"REG: Icon extraction failed for '", displayNameW, L"' (Path: '", utils::Utf8ToWide(prog.iconPath).c_str(), L"', Index: ", prog.iconIndex, L")");
                                }
                            }
                            else
                            {
                                DebugOutput(L"REG INFO: No valid icon path/index for '", displayNameW, L"'. No icon extracted.");
                            }

                            DebugOutput(L"REG: Adding Program: Name='", displayNameW,
                                        L"', ExecPath='", utils::Utf8ToWide(prog.executablePath).c_str(),
                                        L"', Args='", argumentsW.c_str(), // Log raw wide args for easier debug
                                        L"', Kind='", utils::Utf8ToWide(prog.kind).c_str(),
                                        L"', Desc='", utils::Utf8ToWide(prog.description).c_str(),
                                        L"', IconPath='", utils::Utf8ToWide(prog.iconPath).c_str(), L"', IconIndex=", prog.iconIndex);
                            programs.push_back(std::move(prog));

                        } // End if (AppKey opened successfully)
                        else
                        {
                            if (appKeyStatus != ERROR_ACCESS_DENIED)
                            { // Don't log expected access denied
                                DebugOutput(L"REG WARN: Failed to open app subkey '", subKeyNameW, L"'. Status: ", appKeyStatus);
                            }
                        }
                    } // End while (enumerating subkeys)
                } // End loop (uninstall registry paths)
            } // End loop (hive roots)
            return programs;
        }

        
    } // End anonymous namespace

    //-----------------------------------------------------------------------------
    // Public API Implementation
    //-----------------------------------------------------------------------------
    std::vector<utils::Program> GetAllPrograms()
    {
        CoInitializer com_guard;
        if (!com_guard.IsInitialized())
            return {};
        static GdiplusInitializer gdiplus_guard;
        if (!gdiplus_guard.IsInitialized())
        { /* Err */
        }

        DebugOutput(L"----- Starting Program Scan -----");
        std::vector<utils::Program> allFoundPrograms;
        allFoundPrograms.reserve(512);
        try
        {
            DebugOutput(L"--- Scanning Registry ---");
            std::vector<utils::Program> regProgs = GetInstalledProgramsFromRegistryInternal();
            DebugOutput(L"--- Finished Registry Scan (Found ", regProgs.size(), L") ---");
            allFoundPrograms.insert(allFoundPrograms.end(), std::make_move_iterator(regProgs.begin()), std::make_move_iterator(regProgs.end()));
            regProgs.clear();
        }
        catch (const std::exception &e)
        {
            DebugOutput(L"Registry Scan Exception: ", utils::Utf8ToWide(e.what()).c_str());
        }
        catch (...)
        {
            DebugOutput(L"Registry Scan Unknown Exception");
        }
        try
        {
            DebugOutput(L"--- Scanning Start Menu ---");
            std::vector<utils::Program> smProgs = GetProgramsFromStartMenuInternal();
            DebugOutput(L"--- Finished Start Menu Scan (Found ", smProgs.size(), L") ---");
            allFoundPrograms.insert(allFoundPrograms.end(), std::make_move_iterator(smProgs.begin()), std::make_move_iterator(smProgs.end()));
            smProgs.clear();
        }
        catch (const std::exception &e)
        {
            DebugOutput(L"Start Menu Scan Exception: ", utils::Utf8ToWide(e.what()).c_str());
        }
        catch (...)
        {
            DebugOutput(L"Start Menu Scan Unknown Exception");
        }
        DebugOutput(L"--- Deduplicating Results (Initial count: ", allFoundPrograms.size(), L") ---");
        std::vector<utils::Program> finalPrograms = utils::DeduplicatePrograms(allFoundPrograms);
        allFoundPrograms.clear(); // Clear the original vector to free memory
        // Add Settings Pages to the final list
        std::vector<utils::Program> settingsPages = getAllSettingsPages();
        finalPrograms.insert(finalPrograms.end(), std::make_move_iterator(settingsPages.begin()), std::make_move_iterator(settingsPages.end()));

        // Add UWP programs to the final list
        // std::vector<utils::Program> uwpPrograms = GetUwpProgramsOnStaThread();
        // finalPrograms.insert(finalPrograms.end(), std::make_move_iterator(uwpPrograms.begin()), std::make_move_iterator(uwpPrograms.end()));
        std::sort(finalPrograms.begin(), finalPrograms.end(), [](const utils::Program &a, const utils::Program &b)
                  { return _stricmp(a.name.c_str(), b.name.c_str()) < 0; });
        DebugOutput(L"--- Sorted Final Program List ---");
        DebugOutput(L"----- Scan Complete. Returning ", finalPrograms.size(), L" unique programs. -----");
        return finalPrograms;
    }

} // namespace ProgramFinder