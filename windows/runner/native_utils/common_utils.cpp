#define NOMINMAX
#include "common_utils.h"
#include <windows.h>
#include <shlwapi.h>    // Path functions, _wcsicmp, PathParseIconLocationW, SearchPathW, PathFindExtension
#include <shlobj.h>     // SHGetKnownFolderPath, FOLDERID_ constants
#include <Shellapi.h>   // CommandLineToArgvW, ExtractIconExW, PrivateExtractIconsW
#include <shlguid.h>    // CLSID_ShellLink, IID_IShellLinkW
#include <combaseapi.h> // CoInitializeEx, CoUninitialize, CoCreateInstance, CoTaskMemFree
#include <propkey.h>    // Added: Required for PKEY_Link_TargetParsingPath
#include <propvarutil.h> // Added: Required for PropVariantToStringAlloc, InitPropVariantFromString, PropVariantClear
#include <shobjidl.h>   // Added: Required for IPropertyStore
#include <map>          // For deduplication map
#include <set>          // For deduplication set
#include <sstream>      // Needed for debug output
#include <cctype>       // For ::towlowerls.h
#include <wingdi.h>     // For GDI objects (related to icons)
#include <wincrypt.h>   // For CryptBinaryToStringA (Base64)
#include <gdiplus.h>    // For GDI+ icon processing (Bitmap, Graphics)
#include <system_error> // For std::error_code
#include <comdef.h>     // For COM error handling
#include <algorithm>    // For std::sort
#include <filesystem>   // Required for fs::path operations in deduplication


#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "uuid.lib")     // For COM error handling
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Propsys.lib") // Added: Required for PropVariant related functions if not implicitly linked

#define PFINDER_DEBUG // Enable debug output
#ifdef PFINDER_DEBUG
#include <debugapi.h>
#include <optional>
#include <iostream>
template <typename... Args>
void DebugOutput(Args&&... args) { std::wstringstream wss; (wss << ... << std::forward<Args>(args)); wss << L"\n"; OutputDebugStringW(wss.str().c_str()); }
#else
#define DebugOutput(...) do {} while (0)
#endif

namespace utils{

// --- Deleters ---
struct ComReleaser { void operator()(IUnknown *p) const { if (p) p->Release(); } };
template <typename T> using ComUniquePtr = std::unique_ptr<T, ComReleaser>;
struct HIconDeleter { void operator()(HICON h) const { if (h) DestroyIcon(h); } };
using HIconUniquePtr = std::unique_ptr<HICON__, HIconDeleter>;
struct RegKeyDeleter { void operator()(HKEY h) const { if(h && h != INVALID_HANDLE_VALUE) RegCloseKey(h); } };
using RegKeyUniquePtr = std::unique_ptr<HKEY__, RegKeyDeleter>;
struct CoTaskMemDeleter { void operator()(void* pv) const { if (pv) CoTaskMemFree(pv); } };
template <typename T> using CoTaskMemUniquePtr = std::unique_ptr<T, CoTaskMemDeleter>;
struct LocalMemDeleter { void operator()(HLOCAL h) const { if (h) LocalFree(h); } };
using LocalMemUniquePtr = std::unique_ptr<void, LocalMemDeleter>;

namespace fs = std::filesystem;
using namespace Gdiplus;


// --- String Conversion Utilities ---
std::string WideToUtf8(const WCHAR *wideStr, int wideStrLen = -1) { if (!wideStr || (wideStrLen == -1 && wideStr[0] == L'\0') || wideStrLen == 0) return ""; int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideStr, wideStrLen, NULL, 0, NULL, NULL); if (sizeNeeded <= 0) { /*Debug*/ return ""; } std::string utf8Str; utf8Str.resize(static_cast<size_t>(sizeNeeded)); int bytesConverted = WideCharToMultiByte(CP_UTF8, 0, wideStr, wideStrLen, &utf8Str[0], sizeNeeded, NULL, NULL); if (bytesConverted <= 0) { /*Debug*/ return ""; } if (wideStrLen == -1) { if (utf8Str.length() > 0 && utf8Str.back() == '\0') { utf8Str.resize(static_cast<size_t>(bytesConverted) - 1); } else { utf8Str.resize(static_cast<size_t>(bytesConverted)); } } else { utf8Str.resize(static_cast<size_t>(bytesConverted)); } return utf8Str; }
std::string WideToUtf8(const std::wstring &wideStr) { if(wideStr.empty()) return ""; return WideToUtf8(wideStr.c_str(), static_cast<int>(wideStr.length())); }
std::wstring Utf8ToWide(const std::string &utf8Str) { if (utf8Str.empty()) return L""; int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), NULL, 0); if (sizeNeeded <= 0) { /*Debug*/ return L""; } std::wstring wideStr; wideStr.resize(sizeNeeded); int charsConverted = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), &wideStr[0], sizeNeeded); if (charsConverted <= 0) { /*Debug*/ return L""; } return wideStr; }

// --- Filesystem Path Conversion & Normalization ---
std::string PathToUtf8(const fs::path& p) { return WideToUtf8(p.wstring()); }
fs::path Utf8ToPath(const std::string& utf8Str) { return fs::path(Utf8ToWide(utf8Str)); }
std::string NormalizePath(const std::string &path_utf8) { if (path_utf8.empty()) return ""; std::wstring wide_path = Utf8ToWide(path_utf8); if (wide_path.empty() && !path_utf8.empty()) { /*Debug*/ return path_utf8; } try { fs::path p(wide_path); fs::path canonical_p = fs::weakly_canonical(p); std::string utf8_canonical_path = WideToUtf8(canonical_p.wstring()); if (utf8_canonical_path.empty() && !canonical_p.empty()) { /*Debug*/ return path_utf8; } return utf8_canonical_path; } catch (const fs::filesystem_error& [[maybe_unused]] e) { /*Debug*/ return path_utf8; } catch (...) { /*Debug*/ return path_utf8; } }

// --- Base64 Encoding Helper ---
std::string Base64Encode(const std::vector<uint8_t>& data) { if (data.empty()) return ""; if (data.size() > std::numeric_limits<DWORD>::max()) { /*Err*/ return ""; } DWORD dataSizeDWORD = static_cast<DWORD>(data.size()); DWORD base64StringSize = 0; if (!CryptBinaryToStringA(data.data(), dataSizeDWORD, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &base64StringSize)) { /*Err*/ return ""; } if (base64StringSize <= 1) { return ""; } std::string base64String; base64String.resize(base64StringSize - 1); DWORD actualSize = base64StringSize; if (!CryptBinaryToStringA(data.data(), dataSizeDWORD, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &base64String[0], &actualSize)) { /*Err*/ return ""; } return base64String; }

// --- GDI+ Encoder CLSID Helper ---
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) { UINT num = 0, size = 0; GetImageEncodersSize(&num, &size); if (size == 0) return -1; std::unique_ptr<ImageCodecInfo, decltype(&free)> pICI((ImageCodecInfo*)(malloc(size)), free); if (!pICI) return -1; GetImageEncoders(num, size, pICI.get()); for (UINT j = 0; j < num; ++j) { if (wcscmp(pICI.get()[j].MimeType, format) == 0) { *pClsid = pICI.get()[j].Clsid; return static_cast<int>(j); } } return -1; }

// --- Icon Extraction and Encoding ---
std::optional<std::string> ExtractAndEncodeIconAsBase64(const std::string &iconPathUtf8, int iconIndex); // Definition unchanged below

// --- Filesystem Path Existence Checks ---
// Use a more permissive check - just needs to exist, not necessarily be a regular file
bool DoesPathExist(const std::wstring& pathW) {
    if (pathW.empty()) return false;
    std::error_code ec;
    try {
         // Use weakly_canonical to resolve symlinks etc. before checking existence
         fs::path p = fs::weakly_canonical(fs::path(pathW), ec);
         if (ec) { // Error during canonicalization itself
             DebugOutput(L"  ExistCheck WARN: weakly_canonical failed for '", pathW.c_str(), L"'. Error: ", Utf8ToWide(ec.message()).c_str());
             // Fallback: check original path
             return fs::exists(fs::path(pathW), ec) && !ec;
         }
         // Check existence of the canonical path
         bool exists = fs::exists(p, ec);
         if (ec) {
             DebugOutput(L"  ExistCheck WARN: fs::exists check failed for '", p.wstring().c_str(), L"'. Error: ", Utf8ToWide(ec.message()).c_str());
             return false;
         }
         return exists;
    } catch (const std::exception& e) {
         DebugOutput(L"  ExistCheck Exception for '", pathW.c_str(), L"': ", Utf8ToWide(e.what()).c_str());
         return false;
    } catch (...) {
         DebugOutput(L"  ExistCheck Unknown Exception for '", pathW.c_str(), L"'");
         return false;
    }
}






std::optional<utils::ShortcutInfo> ResolveShortcut(const fs::path &linkPathFs) {
    HRESULT hr;
    ComUniquePtr<IShellLinkW> psl;
    ComUniquePtr<IPersistFile> ppf;

    const std::wstring linkPathW = linkPathFs.wstring();
    if (linkPathW.empty()) {
        DebugOutput(L"LNK ERR V5: Empty link path provided."); // V5 for version tracking
        return std::nullopt;
    }

    std::wstring linkStemW = linkPathFs.stem().wstring();
    if (linkStemW.empty()) {
        DebugOutput(L"LNK WARN V5: Could not get stem for link '%ls'. Heuristic check disabled.", linkPathW.c_str());
    }

    // --- Load the Link ---
    IShellLinkW *pslRaw = nullptr;
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<LPVOID*>(&pslRaw));
    if (FAILED(hr)) { DebugOutput(L"LNK ERR V5: CoCreateInstance failed H=0x%X", hr); return std::nullopt; }
    psl.reset(pslRaw);

    IPersistFile *ppfRaw = nullptr;
    hr = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void **>(&ppfRaw));
    if (FAILED(hr)) { DebugOutput(L"LNK ERR V5: QueryInterface(IPersistFile) failed H=0x%X", hr); return std::nullopt; }
    ppf.reset(ppfRaw);

    hr = ppf->Load(linkPathW.c_str(), STGM_READ);
    if (FAILED(hr)) { DebugOutput(L"LNK ERR V5: Failed to Load '%ls', H=0x%X", linkPathW.c_str(), hr); return std::nullopt; }

    utils::ShortcutInfo info; // Defaults: iconIndex = -1, iconDataBase64 = "", descriptionUtf8 = "", isFallbackPath = false
    std::wstring finalResolvedPathW;
    bool isUwpApp = false;

    // --- UWP App Check ---
    ComUniquePtr<IPropertyStore> pps;
    IPropertyStore *ppsRaw = nullptr;
    hr = psl->QueryInterface(IID_IPropertyStore, reinterpret_cast<void **>(&ppsRaw));
    if (SUCCEEDED(hr)) {
        pps.reset(ppsRaw); // Manage lifetime with RAII
        PROPVARIANT propVar;
        PropVariantInit(&propVar);
        hr = pps->GetValue(PKEY_Link_TargetParsingPath, &propVar);
        if (SUCCEEDED(hr)) {
            if (propVar.vt == VT_LPWSTR && propVar.pwszVal && propVar.pwszVal[0] != L'\0') {
                // Found AUMID - this is likely a UWP app shortcut
                finalResolvedPathW = propVar.pwszVal;
                isUwpApp = true;
                info.isFallbackPath = false; // UWP AUMID is the intended target
                DebugOutput(L"LNK INFO V5: Detected UWP App via PKEY_Link_TargetParsingPath: '%ls'", finalResolvedPathW.c_str());
            } else {
                 DebugOutput(L"LNK INFO V5: PKEY_Link_TargetParsingPath found but type is not VT_LPWSTR or empty (vt=%u). Proceeding with standard resolution.", propVar.vt);
            }
            PropVariantClear(&propVar); // Clear the variant regardless of type
        } else {
             DebugOutput(L"LNK INFO V5: Failed to get PKEY_Link_TargetParsingPath (H=0x%X). Proceeding with standard resolution.", hr);
        }
        // pps is released automatically by ComUniquePtr destructor
    } else {
        DebugOutput(L"LNK INFO V5: Failed to query IPropertyStore (H=0x%X). Proceeding with standard resolution.", hr);
    }
    // --- End UWP App Check ---


    // --- Standard Shortcut Resolution (if not UWP) ---
    if (!isUwpApp) {
        DebugOutput(L"LNK INFO V5: Attempting standard resolution for '%ls'", linkPathW.c_str());
        // Resolve (Best Effort)
        hr = psl->Resolve(NULL, SLR_NO_UI | SLR_NOUPDATE | SLR_NOSEARCH | SLR_NOLINKINFO);
        if (FAILED(hr)) {
            DebugOutput(L"LNK WARN V5: Resolve() failed for '%ls', H=0x%X (Target might be missing/inaccessible). Continuing...", linkPathW.c_str(), hr);
        }

        WCHAR targetPathRawW[MAX_PATH + 1] = {0};
        HRESULT hrPath = psl->GetPath(targetPathRawW, ARRAYSIZE(targetPathRawW), NULL, SLGP_UNCPRIORITY);
        DebugOutput(L"  Raw Path (Standard): %ls (H=0x%X)", (SUCCEEDED(hrPath) && targetPathRawW[0] ? targetPathRawW : L"<Failed/Empty>"), hrPath);

        // --- Determine Final Target Path (Standard -> Heuristic -> Fallback) ---
        // ... (Keep the existing Target Path Logic exactly as it was, operating on targetPathRawW) ...
        if (SUCCEEDED(hrPath) && targetPathRawW[0] != L'\0') {
            LPCWSTR extensionPtr = PathFindExtensionW(targetPathRawW);
            bool isExecutableExt = false;
            // Simplified check: Does it have an extension? More robust check might be needed.
            if (extensionPtr != nullptr && extensionPtr[0] == L'.' && extensionPtr[1] != L'\0') {
                // Basic check for common executable types (add more if needed)
                if (_wcsicmp(extensionPtr, L".exe") == 0 || _wcsicmp(extensionPtr, L".com") == 0 || _wcsicmp(extensionPtr, L".bat") == 0 || _wcsicmp(extensionPtr, L".cmd") == 0) {
                    isExecutableExt = true;
                }
            }

            // 1. Standard Check
            if (isExecutableExt) {
                if (utils::DoesPathExist(targetPathRawW)) {
                     finalResolvedPathW = targetPathRawW; info.isFallbackPath = false;
                     DebugOutput(L"  Target Logic (Std): Standard - Accepted executable path: '%ls'", finalResolvedPathW.c_str());
                } else { DebugOutput(L"  Target Logic (Std): Standard - Executable path '%ls' does NOT exist.", targetPathRawW); }
            }

            // 2. Heuristic Check (Example: Installer in Windows dir pointing to actual exe)
            if (finalResolvedPathW.empty() && !linkStemW.empty()) {
                 bool isInstallerTypeExt = false;
                 if (extensionPtr != nullptr && (_wcsicmp(extensionPtr, L".msi") == 0 || _wcsicmp(extensionPtr, L".msp") == 0)) {
                     isInstallerTypeExt = true;
                 }
                 bool isInInstallerDir = false; // Simplified check - refine if needed
                 if (std::wstring(targetPathRawW).find(L"\\Installer\\") != std::wstring::npos) {
                     isInInstallerDir = true;
                 }

                 if (isInstallerTypeExt && isInInstallerDir) {
                    DebugOutput(L"  Target Logic (Std): Heuristic - Potential installer link. Searching for '%ls.exe'.", linkStemW.c_str());
                    std::wstring exeToSearch = linkStemW + L".exe"; WCHAR foundPathW[MAX_PATH + 1] = {0};
                    // SearchPathW might be too broad; consider more targeted search if needed
                    DWORD searchResult = SearchPathW(NULL, exeToSearch.c_str(), NULL, ARRAYSIZE(foundPathW), foundPathW, NULL);
                    if (searchResult > 0 && searchResult < ARRAYSIZE(foundPathW)) {
                         if (utils::DoesPathExist(foundPathW)) {
                            finalResolvedPathW = foundPathW; info.isFallbackPath = false;
                            DebugOutput(L"  Target Logic (Std): Heuristic - Success! Found and verified: '%ls'", finalResolvedPathW.c_str());
                         } else { DebugOutput(L"  Target Logic (Std): Heuristic - SearchPathW found '%ls', but it does NOT exist.", foundPathW); }
                    } else { DebugOutput(L"  Target Logic (Std): Heuristic - Failed. SearchPathW could not find '%ls'.", exeToSearch.c_str()); }
                 }
            }

            // 3. Fallback Check
            if (finalResolvedPathW.empty()) {
                // Use the raw path from GetPath only if it exists, regardless of extension
                if (utils::DoesPathExist(targetPathRawW)) {
                     DebugOutput(L"  Target Logic (Std): Fallback - Using existing raw path '%ls' as target.", targetPathRawW);
                     finalResolvedPathW = targetPathRawW; info.isFallbackPath = true; // <<< SET THE FALLBACK FLAG
                } else { DebugOutput(L"  Target Logic (Std): Fallback - Raw path '%ls' does NOT exist.", targetPathRawW); }
            }
        } else { DebugOutput(L"  Target Logic (Std): Rejected - IShellLink::GetPath failed or returned empty. H=0x%X", hrPath); }
        // ... (End of Target Path Logic) ...
    }
    // --- End Standard Shortcut Resolution ---


    // --- Final Decision: Check if any target path was determined ---
    if (finalResolvedPathW.empty()) {
        DebugOutput(L"LNK FAIL V5: No valid target path found for '%ls'.", linkPathW.c_str());
        return std::nullopt;
    }

    // --- Populate ShortcutInfo (Target, Arguments, Description) ---
    // Normalize path unless it's a UWP AUMID (heuristic: contains '!')
    if (isUwpApp && finalResolvedPathW.find(L'!') != std::wstring::npos) {
        info.resolvedTargetPathUtf8 = utils::WideToUtf8(finalResolvedPathW);
        DebugOutput(L"  Target Store: Storing potential AUMID target path without normalization.");
    } else {
        info.resolvedTargetPathUtf8 = utils::NormalizePath(utils::WideToUtf8(finalResolvedPathW));
        DebugOutput(L"  Target Store: Storing normalized target path.");
    }

    if (info.resolvedTargetPathUtf8.empty() && !finalResolvedPathW.empty()) {
        DebugOutput(L"LNK FAIL V5: UTF8 conversion/normalization failed for Resolved Target Path '%ls'.", finalResolvedPathW.c_str());
        return std::nullopt;
    }

    // --- Get Arguments and Description (Common to both UWP and Standard) ---
    WCHAR argumentsRawW[INFOTIPSIZE + 1] = {0};
    WCHAR descriptionRawW[INFOTIPSIZE + 1] = {0};
    HRESULT hrArgs = psl->GetArguments(argumentsRawW, ARRAYSIZE(argumentsRawW));
    HRESULT hrDesc = psl->GetDescription(descriptionRawW, ARRAYSIZE(descriptionRawW));

    DebugOutput(L"  Raw Args: %ls (H=0x%X)", (SUCCEEDED(hrArgs) && argumentsRawW[0] ? argumentsRawW : L"<Failed/Empty>"), hrArgs);
    DebugOutput(L"  Raw Desc: %ls (H=0x%X)", (SUCCEEDED(hrDesc) && descriptionRawW[0] ? descriptionRawW : L"<Failed/Empty>"), hrDesc);

    if (SUCCEEDED(hrArgs) && argumentsRawW[0] != L'\0') {
        info.argumentsUtf8 = utils::WideToUtf8(argumentsRawW);
        if (info.argumentsUtf8.empty() && argumentsRawW[0] != L'\0') {
            DebugOutput(L"LNK WARN V5: UTF8 conversion failed for arguments '%ls'. Args cleared.", argumentsRawW);
            info.argumentsUtf8.clear();
        }
    } else {
        info.argumentsUtf8.clear();
    }

    if (SUCCEEDED(hrDesc) && descriptionRawW[0] != L'\0') {
        info.descriptionUtf8 = utils::WideToUtf8(descriptionRawW);
        if (info.descriptionUtf8.empty() && descriptionRawW[0] != L'\0') {
             DebugOutput(L"LNK WARN V5: UTF8 conversion failed for description '%ls'. Description cleared.", descriptionRawW);
             info.descriptionUtf8.clear();
        }
    } else {
        info.descriptionUtf8.clear();
    }

    DebugOutput(L"  Stored Target: '%s' (UWP=%s, Fallback=%s)", info.resolvedTargetPathUtf8.c_str(), isUwpApp ? "true" : "false", info.isFallbackPath ? "true" : "false");
    DebugOutput(L"  Stored Args: '%s'", info.argumentsUtf8.c_str());
    DebugOutput(L"  Stored Desc: '%s'", info.descriptionUtf8.c_str());


    // --- Determine Final Icon Path and Index (Common to both UWP and Standard) ---
    WCHAR iconLocationRawW[MAX_PATH + 1] = {0};
    int retrievedIconIndex = 0;
    HRESULT hrIcon = psl->GetIconLocation(iconLocationRawW, ARRAYSIZE(iconLocationRawW), &retrievedIconIndex);
    DebugOutput(L"  Raw Icon: %ls [%d] (H=0x%X)", (SUCCEEDED(hrIcon) && iconLocationRawW[0] ? iconLocationRawW : L"<Failed/Empty>"), retrievedIconIndex, hrIcon);

    std::wstring finalIconPathW;
    int finalIconIndex = -1;
    // ... (Keep the existing Icon Path/Index Logic exactly as it was, but be aware the default might be an AUMID) ...
    if (SUCCEEDED(hrIcon) && iconLocationRawW[0] != L'\0') {
        WCHAR expandedIconPathW[MAX_PATH * 2] = {0}; // Increased buffer size for safety
        DWORD expandedLen = ExpandEnvironmentStringsW(iconLocationRawW, expandedIconPathW, ARRAYSIZE(expandedIconPathW));
        const WCHAR* pathToCheck = (expandedLen > 0 && expandedLen < ARRAYSIZE(expandedIconPathW)) ? expandedIconPathW : iconLocationRawW;

        // For UWP, the icon path might *be* the AUMID itself, which DoesPathExist will fail.
        // Or it might be a resource path like '@{AppPath}\Assets\icon.png'.
        // We keep the DoesPathExist check for standard files. If it fails, we fall back.
        if (utils::DoesPathExist(pathToCheck)) {
            finalIconPathW = pathToCheck; finalIconIndex = retrievedIconIndex;
            DebugOutput(L"  Icon Logic: Using specific icon path (exists): '%ls' [%d]", finalIconPathW.c_str(), finalIconIndex);
        } else {
            // If path doesn't exist, maybe it's a UWP resource path or AUMID?
            // Let's trust the path if it looks like a resource path or contains '!', otherwise default.
            // This is a simple heuristic. A more robust solution might involve parsing PRI resources.
            if (std::wstring(pathToCheck).find(L'!') != std::wstring::npos || // Looks like AUMID
                std::wstring(pathToCheck).find(L"\\Assets\\") != std::wstring::npos ||
                std::wstring(pathToCheck).find(L"\\images\\") != std::wstring::npos ||
                std::wstring(pathToCheck).find(L".png") != std::wstring::npos ||
                std::wstring(pathToCheck).find(L".ico") != std::wstring::npos ||
                (pathToCheck[0] == L'@' && pathToCheck[1] == L'{')) // Starts with @{...}
            {
                 finalIconPathW = pathToCheck; finalIconIndex = retrievedIconIndex;
                 DebugOutput(L"  Icon Logic: Assuming resource/AUMID path (doesn't exist directly): '%ls' [%d]", finalIconPathW.c_str(), finalIconIndex);
            } else {
                 DebugOutput(L"  Icon Logic: Specific icon path '%ls' does NOT exist or isn't recognized resource/AUMID. Will default.", pathToCheck);
            }
        }
    } else { DebugOutput(L"  Icon Logic: GetIconLocation failed or empty path. Will default."); }

    if (finalIconPathW.empty()) {
        // Default to the RESOLVED target (which could be AUMID for UWP)
        finalIconPathW = finalResolvedPathW;
        finalIconIndex = 0; // Default index for target path is usually 0
        DebugOutput(L"  Icon Logic: Defaulting icon path to resolved target: '%ls' [0]", finalIconPathW.c_str());
    }
    // Ensure index is non-negative if path is set
    if (finalIconIndex < 0 && !finalIconPathW.empty()) { finalIconIndex = 0; }
    // ... (End of Icon Path/Index Logic) ...


    // Store Icon Path/Index
    if (!finalIconPathW.empty()) {
        // Normalize the path unless it's potentially a UWP AUMID or resource path
        // Heuristic: if it contains '!' or starts with '@', or has no slashes (could be AUMID)
        if ((finalIconPathW.find(L'!') != std::wstring::npos || finalIconPathW[0] == L'@' || (finalIconPathW.find(L'\\') == std::wstring::npos && finalIconPathW.find(L'/') == std::wstring::npos)) ) {
             info.iconPathUtf8 = utils::WideToUtf8(finalIconPathW); // Don't normalize potential AUMID/resource ID
             DebugOutput(L"  Icon Store: Storing potential AUMID/resource icon path without normalization.");
        } else {
             info.iconPathUtf8 = utils::NormalizePath(utils::WideToUtf8(finalIconPathW));
             DebugOutput(L"  Icon Store: Storing normalized icon path.");
        }

        if (info.iconPathUtf8.empty() && !finalIconPathW.empty()) {
            DebugOutput(L"LNK WARN V5: UTF8 conversion/normalization failed for Icon Path '%ls'. Icon info cleared.", finalIconPathW.c_str());
            info.iconPathUtf8.clear(); info.iconIndex = -1;
        } else { info.iconIndex = finalIconIndex; }
    } else { info.iconPathUtf8.clear(); info.iconIndex = -1; }
    DebugOutput(L"  Stored Icon: Path='%s' Index=%d", info.iconPathUtf8.c_str(), info.iconIndex);

    // --- Extract Icon Data (Optional, kept as is) ---
    // This might fail more often for UWP apps if the icon path is an AUMID or resource path
    // that ExtractAndEncodeIconAsBase64 doesn't handle directly.
    if (!info.iconPathUtf8.empty() && info.iconIndex >= 0) {
        auto encodedIconOpt = utils::ExtractAndEncodeIconAsBase64(info.iconPathUtf8, info.iconIndex);
        if (encodedIconOpt) {
            info.iconDataBase64 = std::move(*encodedIconOpt);
            // Debug output for success is inside ExtractAndEncodeIconAsBase64
        } else { DebugOutput(L"  Icon Extraction: Failed for '%s' [%d]", info.iconPathUtf8.c_str(), info.iconIndex); }
    }

    // --- Final Success Message ---
    DebugOutput(L"LNK SUCCESS V5: Resolved '%ls' -> Target='%s', Args='%s', Desc='%s', Icon='%s'[%d], UWP=%s, IsFallback=%s",
               linkPathW.c_str(),
               info.resolvedTargetPathUtf8.c_str(),
               info.argumentsUtf8.c_str(),
               info.descriptionUtf8.c_str(),
               info.iconPathUtf8.c_str(),
               info.iconIndex,
               isUwpApp ? L"true" : L"false",
               info.isFallbackPath ? L"true" : L"false");

    // psl and ppf are released automatically by ComUniquePtr destructors
    return info;
}

// --- Deduplication Logic ---
struct StringCompareCaseInsensitive { bool operator()(const std::string& lhs, const std::string& rhs) const { return _stricmp(lhs.c_str(), rhs.c_str()) < 0; } };

// Key: Case-insensitive Filename, Case-sensitive Arguments
using ProgramFilenameArgsKey = std::pair<std::string, std::string>;
struct ProgramFilenameArgsKeyCompare {
    bool operator()(const ProgramFilenameArgsKey &l, const ProgramFilenameArgsKey &r) const {
        int fc = _stricmp(l.first.c_str(), r.first.c_str()); // Case-insensitive filename
        if (fc != 0) return fc < 0;
        return l.second < r.second; // Case-sensitive args
    }
};

// Key: Case-insensitive Filename, Case-insensitive Name
using ProgramFilenameNameKey = std::pair<std::string, std::string>;
struct ProgramFilenameNameKeyCompare {
    bool operator()(const ProgramFilenameNameKey &l, const ProgramFilenameNameKey &r) const {
        int fc = _stricmp(l.first.c_str(), r.first.c_str()); // Case-insensitive filename
        if (fc != 0) return fc < 0;
        return _stricmp(l.second.c_str(), r.second.c_str()) < 0; // Case-insensitive name
    }
};

std::vector<Program> DeduplicatePrograms(std::vector<Program> &allPrograms) {
    // Revised Deduplication Logic (v3 - Filename based)

    if (allPrograms.empty()) {
        return {};
    }

    DebugOutput(L"Deduplicate V3: Starting with ", allPrograms.size(), L" raw program entries.");

    // 1. Define Source Preference Order (Same as V2)
    const std::vector<std::string> sourcePreference = {
        "Start Menu (User)",
        "Start Menu (Common)",
        "Registry (HKCU) Uninstall",
        "Registry (HKLM) Uninstall"
    };
    auto get_source_priority = [&](const std::string& source) -> size_t {
        for (size_t i = 0; i < sourcePreference.size(); ++i) {
            if (source == sourcePreference[i]) return i;
        }
        return sourcePreference.size();
    };

    // 2. Sort programs based on preference (Same as V2)
    std::sort(allPrograms.begin(), allPrograms.end(),
        [&](const Program& a, const Program& b) {
            size_t priorityA = get_source_priority(a.source);
            size_t priorityB = get_source_priority(b.source);
            if (priorityA != priorityB) {
                return priorityA < priorityB; // Higher priority (lower index) comes first
            }
            // Secondary sort: Prefer entries with icon data if priority is the same
            bool aHasIcon = !a.iconDataBase64.empty();
            bool bHasIcon = !b.iconDataBase64.empty();
            if (aHasIcon != bHasIcon) {
                return aHasIcon; // True (a has icon, b doesn't) comes before false
            }
            return false; // Keep original relative order otherwise
        });

    DebugOutput(L"Deduplicate V3: Sorted programs by source preference.");

    // 3. Initialize tracking sets (using filename keys) and result vector
    std::set<ProgramFilenameArgsKey, ProgramFilenameArgsKeyCompare> seenFilenameArgs;
    std::set<ProgramFilenameNameKey, ProgramFilenameNameKeyCompare> seenFilenameName;
    std::vector<Program> uniqueProgramsResult;
    uniqueProgramsResult.reserve(allPrograms.size());

    // 4. Iterate and Filter
    for (Program &p : allPrograms) {
        // Skip entries without a valid executable path early
        if (p.executablePath.empty()) {
            DebugOutput(L"  Skipping (Empty Path): '", Utf8ToWide(p.name).c_str(), L"'");
            continue;
        }

        // Extract filename
        std::string filenameUtf8 = "<NoFilename>";
        try {
            fs::path execPath = Utf8ToPath(p.executablePath);
            if (execPath.has_filename()) {
                filenameUtf8 = PathToUtf8(execPath.filename());
                if (filenameUtf8.empty() && !execPath.filename().empty()) {
                    filenameUtf8 = "<ConvErrFilename>"; // Handle potential conversion error
                }
            }
        } catch (const std::exception& e) {
            filenameUtf8 = "<ExceptionFilename>";
            DebugOutput(L"  Filename Extraction Error for path '", Utf8ToWide(p.executablePath).c_str(), L"': ", Utf8ToWide(e.what()).c_str());
        } catch (...) {
             filenameUtf8 = "<UnknownExceptionFilename>";
             DebugOutput(L"  Unknown Filename Extraction Error for path '", Utf8ToWide(p.executablePath).c_str(), L"'");
        }

        if (filenameUtf8.empty() || filenameUtf8.rfind("<", 0) == 0) { // Check for empty or error placeholders
             DebugOutput(L"  Skipping (Invalid Filename '", Utf8ToWide(filenameUtf8).c_str(), L"'): '", Utf8ToWide(p.name).c_str(), L"' Path='", Utf8ToWide(p.executablePath).c_str(), L"'");
             continue;
        }


        // Create keys for the current program using the filename
        ProgramFilenameArgsKey filenameArgsKey = {filenameUtf8, p.arguments};
        ProgramFilenameNameKey filenameNameKey = {filenameUtf8, p.name};

        // Check if either key combination has been seen before
        bool filenameArgsSeen = seenFilenameArgs.count(filenameArgsKey) > 0;
        bool filenameNameSeen = seenFilenameName.count(filenameNameKey) > 0;

        if (filenameArgsSeen || filenameNameSeen) {
            // Duplicate found based on at least one criterion
            std::wstring reason = L"";
            if (filenameArgsSeen && filenameNameSeen) reason = L"Filename+Args AND Filename+Name";
            else if (filenameArgsSeen) reason = L"Filename+Args";
            else reason = L"Filename+Name";

            DebugOutput(L"  Skipping (Duplicate ", reason, L"): '", Utf8ToWide(p.name).c_str(), L"' Filename='", Utf8ToWide(filenameUtf8).c_str(), L"'");
            continue; // Skip this program
        } else {
            // This program is unique based on both criteria so far. Keep it.
            DebugOutput(L"  Adding (Unique): '", Utf8ToWide(p.name).c_str(), L"' Filename='", Utf8ToWide(filenameUtf8).c_str(), L"' Source='", Utf8ToWide(p.source).c_str(), L"'");
            // Add its keys to the tracking sets
            seenFilenameArgs.insert(filenameArgsKey);
            seenFilenameName.insert(filenameNameKey);
            // Move the program to the result vector
            uniqueProgramsResult.push_back(std::move(p));
        }
    }

    DebugOutput(L"Deduplicate V3: Finished. ", uniqueProgramsResult.size(), L" unique programs selected.");
    return uniqueProgramsResult;
}


std::optional<std::string> ExtractAndEncodeIconAsBase64(const std::string& iconPathUtf8, int iconIndex) {
    // Ensure GDI+ is initialized for this scope/thread if not done globally
    // static GdiplusInitializer gdiplusInit; // Use static if called repeatedly, or instance if one-off

    // --- Input Validation ---
    if (iconPathUtf8.empty() || iconIndex < 0) {
        DebugOutput(L"Icon Extract Fail: Invalid input path or index.");
        return std::nullopt;
    }

    std::wstring wideIconPath = Utf8ToWide(iconPathUtf8);
    if (wideIconPath.empty() && !iconPathUtf8.empty()) {
        DebugOutput(L"Icon Extract Fail: UTF-8 to Wide conversion failed for path: ", iconPathUtf8.c_str());
        return std::nullopt;
    }

    // --- Icon Extraction Attempts ---
    HICON hIconRaw = nullptr; // The raw handle extracted

    // Attempt 1: PrivateExtractIconsW (Often gets higher quality/specific size)
    HICON hIconArray[1] = { nullptr };
    UINT iconsExtracted = PrivateExtractIconsW(wideIconPath.c_str(), iconIndex, 256, 256, hIconArray, nullptr, 1, LR_DEFAULTCOLOR);
    if (iconsExtracted >= 1 && hIconArray[0]) { // >= 1 for safety, though 1 is expected
        hIconRaw = hIconArray[0];
        DebugOutput(L"Icon Info: Extracted via PrivateExtractIconsW for '", wideIconPath, L"' [", iconIndex, L"]");
    } else {
        // Attempt 2: ExtractIconExW (Specific Index)
        HICON hLarge = nullptr, hSmall = nullptr;
        // ExtractIconExW returns the *total* number of icons, or UINT_MAX on error.
        iconsExtracted = ExtractIconExW(wideIconPath.c_str(), iconIndex, &hLarge, &hSmall, 1);

        if (iconsExtracted > 0 && iconsExtracted != UINT_MAX) {
            hIconRaw = (hLarge != nullptr) ? hLarge : hSmall; // Prefer large if available
            // Clean up the unused handle *immediately*
            if (hIconRaw == hLarge && hSmall != nullptr) { DestroyIcon(hSmall); hSmall = nullptr;}
            else if (hIconRaw == hSmall && hLarge != nullptr) { DestroyIcon(hLarge); hLarge = nullptr; }
            DebugOutput(L"Icon Info: Extracted via ExtractIconExW (Index ", iconIndex, L") for '", wideIconPath, L"'");
        } else {
            // Cleanup any handles potentially returned even on failure count
            if(hLarge) DestroyIcon(hLarge);
            if(hSmall) DestroyIcon(hSmall);
             hLarge = nullptr; hSmall = nullptr; // Reset for clarity

            // Attempt 3: ExtractIconExW (Index 0 Fallback, if requested index wasn't 0)
            if (iconIndex != 0) {
                iconsExtracted = ExtractIconExW(wideIconPath.c_str(), 0, &hLarge, &hSmall, 1);
                 if (iconsExtracted > 0 && iconsExtracted != UINT_MAX) {
                     hIconRaw = (hLarge != nullptr) ? hLarge : hSmall;
                     if (hIconRaw == hLarge && hSmall != nullptr) { DestroyIcon(hSmall); hSmall = nullptr; }
                     else if (hIconRaw == hSmall && hLarge != nullptr) { DestroyIcon(hLarge); hLarge = nullptr; }
                     DebugOutput(L"Icon Info: Extracted via ExtractIconExW (Index 0 Fallback) for '", wideIconPath, L"'");
                 } else {
                     if(hLarge) DestroyIcon(hLarge);
                     if(hSmall) DestroyIcon(hSmall);
                 }
            }
        }
    }

    // Attempt 4 & 5: SHGetFileInfoW (Fallback for associated icons)
    if (!hIconRaw) {
        DebugOutput(L"Icon Info: Trying SHGetFileInfoW fallback for '", wideIconPath, L"'");
        SHFILEINFOW sfi = {0};
        // Try Large Icon first
        UINT flags = SHGFI_ICON | SHGFI_USEFILEATTRIBUTES | SHGFI_LARGEICON;
        // Note: SHGetFileInfoW returns a *handle that needs to be destroyed*
        if (SHGetFileInfoW(wideIconPath.c_str(), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), flags) != 0 && sfi.hIcon) {
            hIconRaw = sfi.hIcon; // Transfer ownership
            DebugOutput(L"Icon Info: Extracted via SHGetFileInfoW (Large) for '", wideIconPath, L"'");
        } else {
             // If large failed or returned null handle, try small
             if(sfi.hIcon) DestroyIcon(sfi.hIcon); // Destroy potentially returned null handle from large attempt if it failed
             flags = SHGFI_ICON | SHGFI_USEFILEATTRIBUTES | SHGFI_SMALLICON;
             if (SHGetFileInfoW(wideIconPath.c_str(), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), flags) != 0 && sfi.hIcon) {
                 hIconRaw = sfi.hIcon; // Transfer ownership
                 DebugOutput(L"Icon Info: Extracted via SHGetFileInfoW (Small) for '", wideIconPath, L"'");
             } else {
                 // Ensure handle is destroyed if small icon call failed but somehow returned one
                  if(sfi.hIcon) DestroyIcon(sfi.hIcon);
             }
        }
    }

    // --- Final Check & Ownership Transfer ---
    if (!hIconRaw) {
        DebugOutput(L"Icon Extract Fail: No icon handle could be obtained for '", wideIconPath, L"' [", iconIndex, L"] after all attempts.");
        return std::nullopt;
    }

    // Transfer ownership to RAII wrapper IMMEDIATELY.
    HIconUniquePtr hIcon(hIconRaw);
    hIconRaw = nullptr; // Prevent any accidental use/deletion of the raw handle

    // --- GDI+ Conversion to PNG ---
    std::vector<uint8_t> pngData;
    // GDI+ operations can throw exceptions or return error statuses.
    try {
        // Convert HICON to GDI+ Bitmap
        std::unique_ptr<Gdiplus::Bitmap> iconGdipBitmap(Gdiplus::Bitmap::FromHICON(hIcon.get()));
        if (!iconGdipBitmap || iconGdipBitmap->GetLastStatus() != Gdiplus::Ok) {
            DebugOutput(L"Icon Convert Fail: Bitmap::FromHICON failed. Status: ", iconGdipBitmap ? iconGdipBitmap->GetLastStatus() : -1);
            return std::nullopt;
        }

        // Get dimensions
        int iW = static_cast<int>(iconGdipBitmap->GetWidth());
        int iH = static_cast<int>(iconGdipBitmap->GetHeight());
        if (iW <= 0 || iH <= 0) {
             DebugOutput(L"Icon Convert Fail: Invalid bitmap dimensions (", iW, L"x", iH, L").");
             return std::nullopt;
        }

        // Create a target bitmap with Alpha channel for transparency preservation
        Gdiplus::Bitmap targetBitmap(iW, iH, PixelFormat32bppARGB);
        if (targetBitmap.GetLastStatus() != Gdiplus::Ok) {
            DebugOutput(L"Icon Convert Fail: Failed to create target Bitmap. Status: ", targetBitmap.GetLastStatus());
            return std::nullopt;
        }

        // Create Graphics object to draw onto the target bitmap
        Gdiplus::Graphics graphics(&targetBitmap);
        if (graphics.GetLastStatus() != Gdiplus::Ok) {
            DebugOutput(L"Icon Convert Fail: Failed to create Graphics object. Status: ", graphics.GetLastStatus());
            return std::nullopt;
        }

        // Draw the original icon bitmap onto the target ARGB bitmap
        graphics.Clear(Gdiplus::Color(0, 0, 0, 0)); // Clear with transparent background
        Gdiplus::Status drawStatus = graphics.DrawImage(iconGdipBitmap.get(), 0, 0, 0, 0, iW, iH, Gdiplus::UnitPixel);
         if (drawStatus != Gdiplus::Ok) {
            DebugOutput(L"Icon Convert Fail: Graphics::DrawImage failed. Status: ", drawStatus);
            return std::nullopt;
         }

        // Get the CLSID for the PNG encoder
        CLSID pngClsid;
        if (GetEncoderClsid(L"image/png", &pngClsid) == -1) {
            DebugOutput(L"Icon Convert Fail: Could not find PNG encoder CLSID.");
            return std::nullopt;
        }

        // --- Save to In-Memory Stream ---
        IStream* pStreamRaw = nullptr;
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, 0); // Allocate memory for the stream
        if (!hGlobal) {
            DebugOutput(L"Icon Convert Fail: GlobalAlloc failed. Error: ", GetLastError());
            return std::nullopt;
        }

        HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStreamRaw); // TRUE means free hGlobal when stream released
        if (FAILED(hr) || !pStreamRaw) {
            DebugOutput(L"Icon Convert Fail: CreateStreamOnHGlobal failed. HRESULT: ", hr);
            GlobalFree(hGlobal); // Must free manually if stream creation failed
            return std::nullopt;
        }
        ComUniquePtr<IStream> streamPtr(pStreamRaw); // Transfer ownership to RAII wrapper
        pStreamRaw = nullptr; // Prevent accidental use

        // Save the bitmap to the stream as PNG
        Gdiplus::Status saveStatus = targetBitmap.Save(streamPtr.get(), &pngClsid, nullptr);
        if (saveStatus != Gdiplus::Ok) {
            DebugOutput(L"Icon Convert Fail: Bitmap::Save to stream failed. Status: ", saveStatus);
            // streamPtr RAII wrapper will handle release and GlobalFree
            return std::nullopt;
        }

        // --- Read PNG Data from Stream ---
        ULARGE_INTEGER streamSize = {0};
        LARGE_INTEGER zeroOffset = {0};
        hr = streamPtr->Seek(zeroOffset, STREAM_SEEK_END, &streamSize);
        if (FAILED(hr) || streamSize.QuadPart == 0 || streamSize.QuadPart > std::numeric_limits<size_t>::max()) {
            DebugOutput(L"Icon Convert Fail: Failed to seek/get stream size. HRESULT: ", hr, L", Size: ", streamSize.QuadPart);
             return std::nullopt;
        }

        size_t pngSize = static_cast<size_t>(streamSize.QuadPart);
        pngData.resize(pngSize);

        hr = streamPtr->Seek(zeroOffset, STREAM_SEEK_SET, nullptr); // Rewind stream
        if (FAILED(hr)) {
            DebugOutput(L"Icon Convert Fail: Failed to seek stream to start. HRESULT: ", hr);
            return std::nullopt;
        }

        ULONG bytesRead = 0;
        hr = streamPtr->Read(pngData.data(), static_cast<ULONG>(pngData.size()), &bytesRead);
        if (FAILED(hr) || bytesRead != pngData.size()) {
            DebugOutput(L"Icon Convert Fail: Failed to read stream data. HRESULT: ", hr, L", Bytes Read: ", bytesRead, L", Expected: ", pngData.size());
            return std::nullopt;
        }
        // Stream is released (and hGlobal freed) automatically by ComUniquePtr destructor now

    } catch (const _com_error& e) {
        DebugOutput(L"Icon Convert Fail: COM Exception caught: ", e.ErrorMessage(), L" HRESULT: ", e.Error());
        return std::nullopt;
    } catch (const std::exception& e) {
        DebugOutput(L"Icon Convert Fail: Standard Exception caught: ", e.what());
        return std::nullopt;
    } catch (...) {
        DebugOutput(L"Icon Convert Fail: Unknown exception caught during GDI+ processing.");
        return std::nullopt;
    }

    // --- Base64 Encode ---
    if (pngData.empty()) {
         DebugOutput(L"Icon Encode Warning: PNG data vector is empty after conversion for '", wideIconPath, L"' [", iconIndex, L"]");
         // Decide if this is an error or just return empty optional
         return std::nullopt; // Treat as failure
    }

    std::string b64Str = Base64Encode(pngData);
    if (b64Str.empty() && !pngData.empty()) {
        DebugOutput(L"Icon Encode Fail: Base64 encoding resulted in an empty string for non-empty PNG data.");
        return std::nullopt;
    }

    // --- Success ---
    DebugOutput(L"Icon Encode Success: '", wideIconPath, L"' [", iconIndex, L"]");
    return std::optional<std::string>(std::move(b64Str));
}

}