#ifndef PROGRAM_FINDER_H
#define PROGRAM_FINDER_H
#include "common_utils.h"
#include <vector>
#include <string>
#include <cstdint> // For uint8_t

namespace ProgramFinder {


    /**
     * @brief Scans the system (Registry, Start Menus) and returns a deduplicated list
     *        of installed programs and shortcuts.
     *
     * @details Initializes COM for the duration of the call. Handles deduplication
     *          based on combined path+args and name+filename logic.
     *
     * @return std::vector<Program> A list of unique programs found. Returns an empty
     *         vector if COM initialization fails or critical errors occur.
     */
    std::vector<utils::Program> GetAllPrograms();

    /**
     * @brief Searches the list of unique programs for entries whose names contain the query string.
     *
     * @details Performs a case-insensitive search. Calls GetAllPrograms() internally.
     *
     * @param query The string to search for within program names.
     * @return std::vector<Program> A list of programs matching the query.
     */
    std::vector<utils::Program> SearchPrograms(const std::string& query);

} // namespace ProgramFinder

#endif // PROGRAM_FINDER_H