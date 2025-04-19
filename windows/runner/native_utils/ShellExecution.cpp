#include "ShellExecution.h"
#include "common_utils.h" // Assuming this contains utils::Utf8ToWide

#include <windows.h>
#include <shellapi.h> // Required for ShellExecuteExW
#include <string>

// Note: No <vector> or <iostream> needed for this simplified version

namespace ShellExecution
{

/**
 * @brief Opens a specified file, URL, or application using the Windows Shell.
 *
 * This function uses ShellExecuteExW for robust Unicode handling.
 *
 * @param itemPathUtf8 The UTF-8 encoded path to the item (file, URL, executable) to open.
 * @param argumentsUtf8 Optional UTF-8 encoded arguments to pass to the item (if it's an executable).
 * @return true if the shell execution was initiated successfully, false otherwise.
 */
bool OpenItem(const std::string& itemPathUtf8, const std::string& argumentsUtf8 = "") // Default empty arguments
{
    if (itemPathUtf8.empty())
    {
        return false; // Cannot open an empty path
    }

    // Convert UTF-8 inputs to Windows native UTF-16 (wide strings)
    std::wstring wItemPath = utils::Utf8ToWide(itemPathUtf8);
    std::wstring wArguments = utils::Utf8ToWide(argumentsUtf8);

    // Check for conversion failures (important for paths, optional for args depending on need)
    if (wItemPath.empty() && !itemPathUtf8.empty()) {
        // Conversion failed for the essential item path
        return false;
    }
    if (wArguments.empty() && !argumentsUtf8.empty()) {
         // Conversion failed for non-empty arguments - fail the operation
         return false;
    }

    SHELLEXECUTEINFOW sei = { sizeof(sei) }; // Use {} for zero-initialization is cleaner
    sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT; // Get process handle, wait for DDE
    sei.hwnd = NULL;                     // No owner window
    sei.lpVerb = L"open";                // Default action
    sei.lpFile = wItemPath.c_str();      // The item to open (UTF-16)
    // Pass arguments only if they are not empty after conversion
    sei.lpParameters = wArguments.empty() ? NULL : wArguments.c_str();
    sei.lpDirectory = NULL;              // Default working directory
    sei.nShow = SW_SHOWNORMAL;           // Show the window normally

    if (ShellExecuteExW(&sei))
    {
        // Successfully launched.
        // Close the process handle if we got one, as we don't need to wait for it.
        if (sei.hProcess != NULL)
        {
            CloseHandle(sei.hProcess);
        }
        return true;
    }
    else
    {
        // Failed to launch. GetLastError() could be used here for diagnostics if needed.
        return false;
    }
}

} // namespace ShellExecution