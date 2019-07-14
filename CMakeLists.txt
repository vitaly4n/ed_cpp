cmake_minimum_required(VERSION 3.10)

project(black_belt)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DLOCAL_TEST)

macro(define_testapp testapp src_list)
    add_definitions(-DTESTING_DIR_${testapp}=${CMAKE_CURRENT_SOURCE_DIR}/tests/${testapp})
    set(${testapp}_DIR ${testapp})
    set(${testapp}_SRC "")
    foreach(src ${src_list})
        list(APPEND ${testapp}_SRC ${${testapp}_DIR}/${src})
    endforeach()
	add_executable(${testapp}_test ${${testapp}_SRC})
endmacro()

define_testapp(banned_domains_sanitize "banned_domains.cpp")
define_testapp(personal_budget_sanitize "personal_budget.cpp")
define_testapp(transport_E_sanitize "descriptions.cpp;json.cpp;main.cpp;requests.cpp;sphere.cpp;transport_catalog.cpp;transport_router.cpp;utils.cpp;tests.cpp")

