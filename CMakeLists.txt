project(red_belt)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

###########################################
## macro log
###########################################

set(macro_LOG_DIR macro_LOG)

set(macro_LOG_SRC
    ${macro_LOG_DIR}/log_main.cpp
)

add_executable(macro_LOG_test ${macro_LOG_SRC})

###########################################
## SORT_BY
###########################################

set(SORT_BY_DIR SORT_BY)

set(SORT_BY_SRC
    ${SORT_BY_DIR}/sort_by_main.cpp
)

add_executable(SORT_BY_test ${SORT_BY_SRC})