#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <guiddef.h>

namespace utils
{

    // --- Internal Structure for Shortcut Data (Includes flag again) ---
    struct ShortcutInfo
    {
        std::string resolvedTargetPathUtf8; // Path determined via resolution (could be exe, heuristic result, or fallback like .ico)
        std::string argumentsUtf8;
        std::string iconPathUtf8; // Path to the icon resource
        int iconIndex = -1;
        std::string iconDataBase64;
        std::string kind=""; // Optional kind (e.g., "shortcut", "executable", etc.)
        std::string descriptionUtf8=""; // Optional description (e.g., from registry)
        bool isFallbackPath = false; // True if resolvedTargetPathUtf8 is the non-ideal path from GetPath (e.g. .ico)
    };

    struct Program
    {
        std::string name;
        std::string executablePath;
        std::string arguments;
        std::string iconPath;
        int iconIndex = -1;
        std::string iconDataBase64;   // BMP icon data
        std::string source;           // Where it was found (Registry, Start Menu)
        std::string description = ""; // Optional description (e.g., from registry)
        std::string kind = "";        // Optional kind (e.g., "shortcut", "executable", etc.)
                                      /*
                                       Possible string values for the System.Kind property (PKEY_Kind)
                                       and their corresponding user-friendly display text:
                              
                                       Format: "internal value" -> User-friendly Text
                              
                                       "calendar"       -> Calendar
                                       "communication"  -> Communication
                                       "contact"        -> Contact
                                       "document"       -> Document
                                       "email"          -> E-mail
                                       "feed"           -> Feed
                                       "folder"         -> Folder
                                       "game"           -> Game
                                       "instantmessage" -> Instant Message
                                       "journal"        -> Journal
                                       "link"           -> Link
                                       "movie"          -> Movie
                                       "music"          -> Music
                                       "note"           -> Note
                                       "picture"        -> Picture
                                       "playlist"       -> Playlist
                                       "program"        -> Program        // <-- Value for program items
                                       "recordedtv"     -> Recorded TV
                                       "searchfolder"   -> Saved Search
                                       "task"           -> Task
                                       "video"          -> Video
                                       "webhistory"     -> Web History
                                       "unknown"        -> Unknown
                              
                                       Note: This property is a 'Multivalue String', meaning an item could potentially
                                       have more than one kind associated with it, though typically one is primary.
                                       For an item classified as a 'Program', the value stored would be "program".
                                      */
        // Default constructor and move constructor/assignment for efficiency
        Program() = default;
        Program(Program &&other) noexcept = default;
        Program &operator=(Program &&other) noexcept = default;

        // Delete copy constructor/assignment as iconData can be large
        Program(const Program &) = delete;
        Program &operator=(const Program &) = delete;
    };

    // --- String Conversion Utilities ---
    std::string WideToUtf8(const wchar_t *wideStr, int wideStrLen);
    std::string WideToUtf8(const std::wstring &wideStr);
    std::wstring Utf8ToWide(const std::string &utf8Str);

    // --- Filesystem Path Conversion & Normalization ---
    std::string PathToUtf8(const std::filesystem::path &p);
    std::filesystem::path Utf8ToPath(const std::string &utf8Str);
    std::string NormalizePath(const std::string &path_utf8);

    // --- Base64 Encoding Helper ---
    std::string Base64Encode(const std::vector<uint8_t> &data);

    // --- GDI+ Encoder CLSID Helper ---
    int GetEncoderClsid(const wchar_t *format, CLSID *pClsid);

    // --- Icon Extraction and Encoding ---
    std::optional<std::string> ExtractAndEncodeIconAsBase64(const std::string &iconPathUtf8, int iconIndex);

    // --- Filesystem Path Existence Checks ---
    bool DoesPathExist(const std::wstring &pathW);

    // --- Shortcut Resolution ---
    std::optional<ShortcutInfo> ResolveShortcut(const std::filesystem::path &linkPathFs);

    // --- Deduplication Logic ---
    std::vector<Program> DeduplicatePrograms(std::vector<Program> &allPrograms);

} // namespace utils

#endif // COMMON_UTILS_H