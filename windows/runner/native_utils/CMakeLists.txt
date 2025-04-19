
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_library(native_utils_lib STATIC
  "winsearch.cpp"
  "ProgramFinder.cpp"
  "ShellExecution.cpp"
  "common_utils.cpp"
  "UwpFinder.cpp"
  "SettingsPages.cpp"
)

target_include_directories(native_utils_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)



