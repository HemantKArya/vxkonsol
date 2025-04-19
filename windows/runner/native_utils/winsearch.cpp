///////////////////////////////////////////////////////////////////////////////
// winsearch.cpp
//
// Implements searching the Windows Search Index, resolving shortcuts,
// extracting high-quality icons (PNG Base64), and returning results
// compatible with the utils::Program structure. Includes deduplication
// based on the initial ItemName found in the index.
///////////////////////////////////////////////////////////////////////////////

// Define NOMINMAX *before* including Windows.h to prevent macro conflicts
#define NOMINMAX

#include "winsearch.h" // Header for this module (Provides utils::Program, etc.)

// --- Minimal Necessary Includes ---

// Standard C++
#include <optional>       // For std::optional
#include <memory>         // For std::unique_ptr
#include <filesystem>     // For std::filesystem (C++17)
#include <map>            // For std::map (deduplication)
#include <wchar.h>        // For _wcsicmp

// Windows & COM Core
#define WIN32_LEAN_AND_MEAN
#include <windows.h>      // Core Windows API
#include <comdef.h>       // For _com_error, _bstr_t, _variant_t (used with #import)
#include <objidl.h>       // For IStream (used by GDI+)

// Shell APIs (mostly for shortcut resolution & icon extraction, often via utils::)
#include <shlobj.h>       // For IShellLink, SHGetKnownFolderPath etc.
#include <shellapi.h>     // For ExtractIconExW
#include <shlguid.h>      // For CLSID_ShellLink, IID_IShellLinkW
#include <knownfolders.h> // For FOLDERID_... GUIDs

// GDI / GDI+ / Crypto (for icon processing & encoding, often via utils::)
#include <wingdi.h>       // GDI objects (HICON, HBITMAP, BITMAPINFO etc.)
#include <wincrypt.h>     // For CryptBinaryToStringA (Base64)
#include <gdiplus.h>      // GDI+ for PNG conversion

// --- Minimal Pragmas and Imports ---

// Link necessary libraries (Hard to avoid in single-file context without build system setup)
#pragma comment(lib, "Ole32.lib")      // COM
#pragma comment(lib, "Shell32.lib")    // Shell functions
#pragma comment(lib, "User32.lib")     // DestroyIcon etc.
#pragma comment(lib, "Gdi32.lib")      // GDI functions
#pragma comment(lib, "Crypt32.lib")    // Crypto functions (Base64)
#pragma comment(lib, "Gdiplus.lib")    // GDI+ functions
#pragma comment(lib, "uuid.lib")       // COM GUIDs

// Import the ADO type library
#import "libid:B691E011-1797-432E-907A-4D8C69339129" version("6.0") rename("EOF", "EndOfFile") no_namespace

// Define _DEBUG if you want debug output via OutputDebugStringW
// #define WINSEARCH_DEBUG

#ifdef WINSEARCH_DEBUG
#include <sstream> // For formatting debug output
template <typename... Args> void DebugOutputWS(Args&&... args) { std::wstringstream wss; (wss << ... << std::forward<Args>(args)); wss << L"\n"; OutputDebugStringW(wss.str().c_str()); }
#else
#define DebugOutputWS(...) // No-op when not debugging
#endif

// Use Gdiplus namespace
using namespace Gdiplus;
namespace fs = std::filesystem;

//-----------------------------------------------------------------------------
// Internal Helpers (Anonymous Namespace)
//-----------------------------------------------------------------------------
namespace {

    // --- RAII Wrappers (Condensed) ---
    struct CoInitializer { HRESULT hr; CoInitializer(); ~CoInitializer(); bool IsInitialized() const; };
    CoInitializer::CoInitializer() : hr(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)) { if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) { DebugOutputWS(L"WinSearchErr: COM Init failed. HRESULT=0x", std::hex, hr); hr = E_FAIL; } else if (hr==S_FALSE || SUCCEEDED(hr)) hr=S_OK; }
    CoInitializer::~CoInitializer() { if (SUCCEEDED(hr)) CoUninitialize(); } bool CoInitializer::IsInitialized() const { return SUCCEEDED(hr); }
    // *** MODIFIED LINE 80: Added {} around DebugOutputWS in GdiplusInitializer constructor ***
    struct GdiplusInitializer { ULONG_PTR t=0; Status s=GenericError; GdiplusInitializer(){ GdiplusStartupInput i; s=GdiplusStartup(&t,&i,0); if(s!=Ok) { DebugOutputWS(L"WinSearchErr: GDI+ Startup failed. Status: ", s); } } ~GdiplusInitializer(){ if(s==Ok&&t!=0) GdiplusShutdown(t); } bool IsInitialized()const{return s==Ok;} };
    struct ComReleaser { void operator()(IUnknown* p) const { if (p) p->Release(); } }; template <typename T> using ComUniquePtr = std::unique_ptr<T, ComReleaser>;
    struct HIconDeleter { void operator()(HICON h) const { if (h) DestroyIcon(h); } }; using HIconUniquePtr = std::unique_ptr<HICON__, HIconDeleter>;
    struct HBitmapDeleter { void operator()(HBITMAP h) const { if (h) DeleteObject(h); } }; using HBitmapUniquePtr = std::unique_ptr<HBITMAP__, HBitmapDeleter>;
    struct CoTaskMemDeleter { void operator()(void* pv) const { if (pv) CoTaskMemFree(pv); } }; template <typename T> using CoTaskMemUniquePtr = std::unique_ptr<T, CoTaskMemDeleter>;

    // --- Case-Insensitive Comparator for Wide Strings ---
    struct CaseInsensitiveLessW { bool operator()(const std::wstring& l, const std::wstring& r) const { return _wcsicmp(l.c_str(), r.c_str()) < 0; } };
   

} // End anonymous namespace

//-----------------------------------------------------------------------------
// Public API Implementation
//-----------------------------------------------------------------------------
 // --- Revised SearchWindowsIndex Function ---
 std::vector<utils::Program> SearchWindowsIndex(const std::string &searchString)
 {
     std::vector<utils::Program> finalResults;

     // Initialize COM and optionally GDI+
     CoInitializer com_guard;
     if (!com_guard.IsInitialized()) {
         // Error already logged by CoInitializer
         return finalResults;
     }
     GdiplusInitializer gdiplus_guard; // Required if icon extraction needs it globally
     if (!gdiplus_guard.IsInitialized()) {
         // Error logged by GdiplusInitializer, maybe proceed without icon extraction?
         // Depending on requirements, you might return here.
     }

     // Convert search string to wide format
     std::wstring searchStringW = utils::Utf8ToWide(searchString);
     if (searchStringW.empty() && !searchString.empty()) {
         DebugOutputWS(L"WinSearchErr: Failed to convert input search string to wide string.");
         return finalResults;
     }

     // Map to store unique items: Key=Path, Value=Tuple{Name, Kind, Comment}
     std::map<std::wstring, std::tuple<std::wstring, std::wstring, std::wstring>, CaseInsensitiveLessW> uniqueRawItems;

     // --- ADO Query Section ---
     _ConnectionPtr pC = nullptr;
     _RecordsetPtr pR = nullptr;
     try {
         HRESULT hr;

         // Create and open ADO connection
         hr = pC.CreateInstance(__uuidof(Connection));
         if (FAILED(hr)) _com_issue_error(hr);
         hr = pC->Open(L"Provider=Search.CollatorDSO;Extended Properties='Application=Windows';", L"", L"", adConnectUnspecified);
         if (FAILED(hr)) _com_issue_error(hr);

         // Construct SQL Query
         std::wstring query = L"SELECT System.ItemName, System.ItemPathDisplay, System.Kind, System.Comment FROM SYSTEMINDEX WHERE ";

         // Escape single quotes in search string for SQL
         std::wstring escapedSearchString = searchStringW;
         size_t pos = 0;
         while ((pos = escapedSearchString.find(L"'", pos)) != std::wstring::npos) {
             escapedSearchString.replace(pos, 1, L"''");
             pos += 2;
         }

         // Build WHERE clause (match name or path, filter by kind)
         if (!escapedSearchString.empty()) {
             query += L"(CONTAINS(System.ItemName,'\"" + escapedSearchString + L"*\"', 1033) OR System.ItemPathDisplay LIKE '%" + escapedSearchString + L"%') AND "; // Added LCID 1033 for English CONTAINS
         }
         query += L"(System.Kind='program' OR System.Kind='link') ";

         DebugOutputWS(L"WinSearch Query: ", query);

         // Create and open ADO recordset
         hr = pR.CreateInstance(__uuidof(Recordset));
         if (FAILED(hr)) _com_issue_error(hr);
         hr = pR->Open(query.c_str(), _variant_t((IDispatch*)pC, true), adOpenStatic, adLockReadOnly, adCmdText);
         if (FAILED(hr)) {
              _com_error e(hr); // Get error details
              DebugOutputWS(L"WinSearchErr: ADO Recordset Open failed H=0x", std::hex, hr, L" Desc:", e.ErrorMessage());
              _com_issue_error(hr); // Rethrow to be caught below
         }

         // --- Process ADO Recordset ---
         if (pR && pR->State == adStateOpen && !pR->EndOfFile)
         {
             FieldsPtr pFields = nullptr;
             hr = pR->get_Fields(&pFields);
              if (FAILED(hr) || !pFields) {
                 DebugOutputWS(L"WS Err: Failed to get Fields collection from Recordset. H=0x", std::hex, hr);
                  _com_issue_error(hr);
              }

             long fieldCount = 0;
             pFields->get_Count(&fieldCount); // Check field count for debugging

             while (!pR->EndOfFile)
             {
                 std::wstring itemNameW, itemPathW, kindW, commentW;
                 _variant_t vName, vPath, vKind, vComment; // Use separate variants

                 // Default Kind for debugging if retrieval fails
                 kindW = L"<Kind Not Retrieved>";

                 try {
                     // Get field values safely using variants
                     vName = pFields->GetItem("System.ItemName")->GetValue();
                     vPath = pFields->GetItem("System.ItemPathDisplay")->GetValue();
                     vKind = pFields->GetItem("System.Kind")->GetValue();
                     vComment = pFields->GetItem("System.Comment")->GetValue();

                     // Extract BSTR values if valid
                     if (vName.vt == VT_BSTR && vName.bstrVal) itemNameW = vName.bstrVal;
                     if (vPath.vt == VT_BSTR && vPath.bstrVal) itemPathW = vPath.bstrVal;

                     // Detailed Kind retrieval and logging
                     kindW = L"<Kind Not Retrieved>"; // Default value

                     if (vKind.vt == VT_BSTR && vKind.bstrVal) {
                         // Handle the simple case: single string
                         kindW = vKind.bstrVal;
                         DebugOutputWS(L"    ADO Retrieved Kind: '", kindW, L"' (VT_BSTR) for item '", itemNameW, L"'");
                     }
                     else if (vKind.vt == (VT_ARRAY | VT_BSTR)) {
                         // Handle the documented case: Multivalue String (SAFEARRAY of BSTR)
                         DebugOutputWS(L"    ADO Retrieved Kind: Type is VT_ARRAY|VT_BSTR for item '", itemNameW, L"'. Processing array...");
                         SAFEARRAY* psa = vKind.parray; // Get the SAFEARRAY pointer from the VARIANT
                         if (psa) {
                             long lBound = 0, uBound = -1;
                             UINT dims = SafeArrayGetDim(psa);
                             HRESULT hrBounds = E_FAIL;

                             if (dims == 1) { // We expect a 1-dimensional array
                                 hrBounds = SafeArrayGetLBound(psa, 1, &lBound); // Get lower bound (usually 0)
                                 if (SUCCEEDED(hrBounds)) {
                                     hrBounds = SafeArrayGetUBound(psa, 1, &uBound); // Get upper bound
                                 }
                             } else {
                                 DebugOutputWS(L"      WARN: Kind SAFEARRAY has ", dims, L" dimensions, expected 1.");
                             }

                             if (SUCCEEDED(hrBounds) && lBound <= uBound) {
                                 // Array is valid and not empty, get the first element
                                 BSTR bstrElement = nullptr;
                                 // Access element at the lower bound (most likely index 0)
                                 HRESULT hrGet = SafeArrayGetElement(psa, &lBound, &bstrElement);
                                 if (SUCCEEDED(hrGet) && bstrElement) {
                                     try {
                                         // Use _bstr_t for automatic BSTR management (SysFreeString)
                                         _bstr_t bstrWrapper(bstrElement, false); // Attach to existing BSTR, wrapper will free it
                                         kindW = (wchar_t*)bstrWrapper; // Assign the string content
                                         DebugOutputWS(L"      Successfully extracted Kind from array [0]: '", kindW, L"'");
                                         // bstrWrapper destructor handles SysFreeString when it goes out of scope
                                     } catch(const _com_error& e) {
                                        (void)e;
                                          kindW = L"<Kind Array BSTR Wrapper Error>";
                                          DebugOutputWS(L"      ERROR: _bstr_t wrapper failed. HRESULT=0x", std::hex, e.Error(), L". Desc: ", e.ErrorMessage());
                                          SysFreeString(bstrElement); // Manually free if wrapper failed
                                     }
                                 } else {
                                     kindW = L"<Kind Array GetElement Failed>";
                                     DebugOutputWS(L"      ERROR: SafeArrayGetElement failed for index ", lBound, L". HRESULT=0x", std::hex, hrGet);
                                     // If GetElement allocated bstrElement but failed, it should be NULL according to docs, but free just in case.
                                     if (bstrElement) SysFreeString(bstrElement);
                                 }
                             } else if (SUCCEEDED(hrBounds)) { // Bounds retrieved successfully, but lBound > uBound
                                 kindW = L"<Kind is Empty Array>";
                                 DebugOutputWS(L"      Kind SAFEARRAY is empty (lBound=", lBound, L", uBound=", uBound, L").");
                             } else { // Failed to get bounds or dimensions were wrong
                                 kindW = L"<Kind Array Bounds/Dim Error>";
                                 DebugOutputWS(L"      ERROR: Failed to get SAFEARRAY bounds or dimensions incorrect. HRESULT=0x", std::hex, hrBounds);
                             }
                             // No need to call SafeArrayDestroy(psa). The _variant_t destructor (VariantClear) handles it.
                         } else {
                              kindW = L"<Kind Array is NULL Ptr>";
                              DebugOutputWS(L"      Kind SAFEARRAY pointer (vKind.parray) is NULL despite VT_ARRAY type.");
                         }
                     }
                     else if (vKind.vt == VT_NULL || vKind.vt == VT_EMPTY) {
                         // Handle NULL or EMPTY variant
                         kindW = L"<Kind is NULL/EMPTY>";
                         DebugOutputWS(L"    ADO Retrieved Kind: NULL or EMPTY (VT=", vKind.vt, L") for item '", itemNameW, L"'");
                     }
                     else {
                         // Handle any other unexpected variant types
                         kindW = L"<Kind is Other Type>";
                         DebugOutputWS(L"    ADO Retrieved Kind: Unexpected Variant Type (VT=", vKind.vt, L") for item '", itemNameW, L"'");
                     }

                     if (vComment.vt == VT_BSTR && vComment.bstrVal) commentW = vComment.bstrVal;

                 } catch (_com_error &fieldError) {
                    (void)fieldError;
                      DebugOutputWS(L"WS Err: Failed to get field value. H=0x", std::hex, fieldError.Error(), L" Desc:", fieldError.ErrorMessage());
                      // Continue to next record if a field fails for one item
                      hr = pR->MoveNext();
                      if (FAILED(hr)) { DebugOutputWS(L"WS Warn: MoveNext failed after field error. H=0x", std::hex, hr); break; }
                      continue;
                 }

                 // Basic validation: Need path and name
                 if (itemPathW.empty() || itemNameW.empty()) {
                      DebugOutputWS(L"WS Warn: Skipping item due to missing Path or Name. Path='", itemPathW, L"', Name='", itemNameW, L"'");
                      hr = pR->MoveNext();
                      if (FAILED(hr)) { DebugOutputWS(L"WS Warn: MoveNext failed after skipping item. H=0x", std::hex, hr); break; }
                      continue;
                 }

                 // Store item in the map (Path -> {Name, Kind, Comment})
                  DebugOutputWS(L"    ADO Storing Tuple: Path='", itemPathW, L"', Name='", itemNameW, L"', Kind='", kindW, L"', Comment='", commentW, L"'");
                 uniqueRawItems.try_emplace(itemPathW, std::make_tuple(itemNameW, kindW, commentW));

                 // Move to the next record
                 hr = pR->MoveNext();
                 if (FAILED(hr)) {
                     DebugOutputWS(L"WS Warn: MoveNext failed. H=0x", std::hex, hr);
                     break; // Exit loop on MoveNext failure
                 }
             } // End while loop
         } // End if recordset is valid and has data

         // Close ADO objects cleanly
         if (pR && pR->State == adStateOpen) pR->Close();
         if (pC && pC->State == adStateOpen) pC->Close();

     } catch (_com_error& e) {
         (void)e; // Suppress unused variable warning
         DebugOutputWS(L"WS Err COM (Outer): H=0x", std::hex, e.Error(), L" Desc:", e.ErrorMessage());
         // Ensure cleanup even on error
         if (pR && pR->State == adStateOpen) try { pR->Close(); } catch(...) {}
         if (pC && pC->State == adStateOpen) try { pC->Close(); } catch(...) {}
     } catch (const std::exception& e) {
         (void)e; // Suppress unused variable warning
         DebugOutputWS(L"WS Err STD (Outer): ", utils::Utf8ToWide(e.what()));
     } catch (...) {
         DebugOutputWS(L"WS Err Unknown (Outer) during ADO");
         // Ensure cleanup even on error
         if (pR && pR->State == adStateOpen) try { pR->Close(); } catch(...) {}
         if (pC && pC->State == adStateOpen) try { pC->Close(); } catch(...) {}
     }

     DebugOutputWS(L"WinSearch: Found ", uniqueRawItems.size(), L" unique raw items to process.");

     // --- PASS 2: Process Unique Items ---
     for (const auto& pair : uniqueRawItems)
     {
         const std::wstring& itemPathDisplayW = pair.first;         // Key: Path
         const auto& itemData = pair.second;                        // Value: Tuple
         const std::wstring& itemNameW = std::get<0>(itemData);     // From Tuple
         const std::wstring& kindW_from_tuple = std::get<1>(itemData); // From Tuple
         const std::wstring& commentW = std::get<2>(itemData);     // From Tuple

         DebugOutputWS(L"WinSearch: Processing Item Path='", itemPathDisplayW, L"', Name='", itemNameW, L"'");
         DebugOutputWS(L"    Processing Tuple Data: Kind='", kindW_from_tuple, L"', Comment='", commentW, L"'");

         utils::Program program; // Create Program object for this item
         try {
             program.source = "Windows Search Index";

             // Assign Name (with UTF-8 conversion)
             program.name = utils::WideToUtf8(itemNameW);
             if (program.name.empty() && !itemNameW.empty()) {
                 DebugOutputWS(L"      WARN: UTF8 conversion failed for Name: '", itemNameW, L"'. Using placeholder.");
                 program.name = "Invalid UTF8 Name";
             }

             // Assign Kind (with UTF-8 conversion and fallback)
             program.kind = utils::WideToUtf8(kindW_from_tuple);
              DebugOutputWS(L"    Assigned program.kind='", utils::Utf8ToWide(program.kind).c_str(), L"' from kindW_from_tuple='", kindW_from_tuple.c_str(), L"'");
             if (program.kind.empty() && !kindW_from_tuple.empty() && kindW_from_tuple.find(L'<') == std::wstring::npos) {
                  DebugOutputWS(L"      WARN: Kind conversion to UTF8 failed or resulted in empty string for '", kindW_from_tuple, L"'.");
             }
             // Set to "unknown" if empty or still contains error markers from retrieval
             if (program.kind.empty() || program.kind.find('<') != std::string::npos) {
                 program.kind = "unknown";
                 DebugOutputWS(L"      Setting kind to 'unknown'.");
             }

             // Assign Initial Description (from index comment, with UTF-8 conversion)
             program.description = utils::WideToUtf8(commentW);
             if (program.description.empty() && !commentW.empty()) {
                  DebugOutputWS(L"      WARN: UTF8 conversion failed for Comment: '", commentW, L"'.");
             }

             // --- Resolve Path/Shortcut ---
             fs::path itemPathFs(itemPathDisplayW);
             std::optional<utils::ShortcutInfo> resolvedInfoOpt;
             bool isLnk = itemPathFs.has_extension() && _wcsicmp(itemPathFs.extension().c_str(), L".lnk") == 0;

             if (isLnk) {
                 // Resolve shortcut (function expected to return description)
                 resolvedInfoOpt = utils::ResolveShortcut(itemPathFs);
                 if (!resolvedInfoOpt) {
                     DebugOutputWS(L"  Skipping: Failed to resolve shortcut '", itemPathDisplayW, L"'");
                     continue; // Skip this item
                 }
                 // Prioritize description from the resolved shortcut if available
                 if (resolvedInfoOpt.has_value() && !resolvedInfoOpt.value().descriptionUtf8.empty()) {
                     DebugOutputWS(L"  Info: Using description from resolved shortcut instead of index comment.");
                     program.description = resolvedInfoOpt.value().descriptionUtf8; // Overwrite
                 }
             } else {
                 // Handle direct paths (non-LNK files, typically Kind='program')
                 std::error_code ec;
                 fs::path canonicalPath = fs::weakly_canonical(itemPathFs, ec); // Try to get full path
                 fs::path finalPathToUse = itemPathFs; // Default to original path

                 if (!ec && fs::exists(canonicalPath, ec)) {
                     finalPathToUse = canonicalPath; // Use canonical if valid and exists
                 } else if (!fs::exists(itemPathFs, ec)) {
                      DebugOutputWS(L"  Skipping: Non-LNK path '", itemPathDisplayW, L"' does not exist.");
                      continue; // Skip if neither original nor canonical exists
                 }
                 // Else: Use original path if canonical failed but original exists

                 // Create 'pseudo' ShortcutInfo
                 utils::ShortcutInfo directInfo;
                 directInfo.resolvedTargetPathUtf8 = utils::NormalizePath(utils::WideToUtf8(finalPathToUse.wstring()));
                 directInfo.argumentsUtf8 = "";
                 directInfo.iconPathUtf8 = directInfo.resolvedTargetPathUtf8; // Default icon to target
                 directInfo.iconIndex = 0;
                 directInfo.isFallbackPath = false;
                 directInfo.descriptionUtf8 = ""; // No separate description for direct items
                 resolvedInfoOpt = directInfo;
             }

             // --- Populate Program from Resolved Info ---
             if (resolvedInfoOpt.has_value()) {
                 const auto& info = resolvedInfoOpt.value();

                 // Check if resolution yielded a valid target path
                 if (info.resolvedTargetPathUtf8.empty()) {
                     DebugOutputWS(L"  Skipping: Resolved info has empty target path for '", itemPathDisplayW, L"'");
                     continue;
                 }

                 // Assign core properties from resolved info
                 program.executablePath = info.resolvedTargetPathUtf8;
                 program.arguments = info.argumentsUtf8;
                 program.iconPath = info.iconPathUtf8;
                 program.iconIndex = info.iconIndex;

                 // Handle LNK fallback: If shortcut resolution used a non-executable target,
                 // set the executable path back to the .lnk file itself for ShellExecute.
                 if (isLnk && info.isFallbackPath) {
                     DebugOutputWS(L"  Adjusting Path: Detected fallback target '", utils::Utf8ToWide(program.executablePath), L"'. Using original LNK path '", itemPathDisplayW, L"' for execution.");
                     program.executablePath = utils::NormalizePath(utils::WideToUtf8(itemPathDisplayW));
                     // Keep arguments from shortcut info, might be relevant for ShellExecute
                 }

                 // Validate/Fallback Icon Path and Index
                 if (program.iconPath.empty() && !program.executablePath.empty()) {
                     program.iconPath = program.executablePath;
                     program.iconIndex = 0;
                 }
                 if (program.iconIndex < 0 && !program.iconPath.empty()) {
                     program.iconIndex = 0;
                 }

                 // Extract Icon Data
                 program.iconDataBase64 = ""; // Clear previous (should be empty anyway)
                 if (!program.iconPath.empty() && program.iconIndex >= 0) {
                     // Call utility function (ensure it's linked/available and handles GDI+ init if needed)
                     auto encodedIconOpt = utils::ExtractAndEncodeIconAsBase64(program.iconPath, program.iconIndex);
                     if (encodedIconOpt) {
                         program.iconDataBase64 = std::move(*encodedIconOpt);
                     } else {
                          DebugOutputWS(L"  Icon extraction failed for '", utils::Utf8ToWide(program.iconPath), L"' [", program.iconIndex, L"]");
                     }
                 } else {
                     DebugOutputWS(L"  Skipping icon extraction (invalid path/index).");
                 }

                 // Final log before adding to results
                 DebugOutputWS(L"  Adding Program: Name='", utils::Utf8ToWide(program.name),
                               L"', Exe='", utils::Utf8ToWide(program.executablePath),
                               L"', Args='", utils::Utf8ToWide(program.arguments),
                               L"', Kind='", utils::Utf8ToWide(program.kind), // Check final kind value
                               L"', Desc='", utils::Utf8ToWide(program.description),
                               L"', Icon='", utils::Utf8ToWide(program.iconPath), L"[", program.iconIndex, L"]'");

                 finalResults.push_back(std::move(program)); // Add the fully populated program
             }

         } catch (const std::exception& e) {
             (void)e; // Suppress unused variable warning
              DebugOutputWS(L"WS Err Proc STD: Item Path='", itemPathDisplayW, L"': ", utils::Utf8ToWide(e.what()));
         } catch (...) {
             DebugOutputWS(L"WS Err Proc UNK: Item Path='", itemPathDisplayW, L"'.");
         }
     } // End Pass 2 loop

     DebugOutputWS(L"WinSearch: Returning ", finalResults.size(), L" processed programs.");
     return finalResults;
 } // End SearchWindowsIndex
