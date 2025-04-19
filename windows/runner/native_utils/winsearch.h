// winsearch.h
#ifndef WINSEARCH_H
#define WINSEARCH_H

#include <string>          // Include for std::string
#include <vector>
#include <cstdint>
#include "common_utils.h"

/**
 * @brief Searches the Windows Search Index for items matching the query,
 *        focusing on programs or program-like entries.
 *
 * @details Uses ADO to query the SYSTEMINDEX. Retrieves item name and path.
 *          Extracts an icon for the item, encodes it as Base64 PNG, and populates
 *          a ProgramFinder::Program structure. Assumes the input query string is UTF-8 encoded.
 *
 * @param searchString The UTF-8 encoded string to search for in item names or paths within the index.
 * @return std::vector<ProgramFinder::Program> A list of programs/items found, matching the
 *         ProgramFinder::Program structure. Returns an empty vector on critical errors (e.g., COM failure).
 */
std::vector<utils::Program> SearchWindowsIndex(const std::string &searchString); // Changed parameter type

// Utility function (can be kept if needed internally or removed if ProgramFinder helpers are used)
// std::string wstringToString(const std::wstring &wstr); // Consider using ProgramFinder's WideToUtf8 instead

#endif // WINSEARCH_H