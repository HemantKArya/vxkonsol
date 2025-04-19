// ShellExecution.h
#ifndef SHELL_EXECUTION_H
#define SHELL_EXECUTION_H

#include <string>

namespace ShellExecution
{
    // Updated signature to accept std::string
    bool OpenItem(const std::string& itemPathUtf8, const std::string& argumentsUtf8);

} // namespace ShellExecution

#endif // SHELL_EXECUTION_H