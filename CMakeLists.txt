cmake_minimum_required(VERSION 3.12)

project(brown_belt)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

macro(define_testapp testapp src_list)
    set(${testapp}_DIR ${testapp})
    set(${testapp}_SRC "")
    foreach(src ${src_list})
        list(APPEND ${testapp}_SRC ${${testapp}_DIR}/${src})
    endforeach()
	add_executable(${testapp}_test ${${testapp}_SRC})
endmacro()

define_testapp(hash_set "hash_set_main.cpp")
define_testapp(set_iterator_increment "set_iterator_increment_main.cpp")
define_testapp(hash_person "hash_person_main.cpp")
define_testapp(secondary_index "secondary_index_main.cpp")
define_testapp(ini "ini_main.cpp;ini.cpp")
define_testapp(restructuring "main.cpp;stats_aggregator.cpp;stats_aggregator_test.cpp")