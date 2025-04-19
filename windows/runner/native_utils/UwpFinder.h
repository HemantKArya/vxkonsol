#ifndef UWP_FINDER_H
#define UWP_FINDER_H
#include <vector>
#include "common_utils.h"

std::vector<utils::Program> GetInstalledUWPPrograms();
std::vector<utils::Program> GetUwpProgramsOnStaThread();

#endif