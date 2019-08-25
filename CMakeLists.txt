cmake_minimum_required(VERSION 3.10)

project(black_belt)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DLOCAL_TEST)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

macro(define_testapp testapp)
    project(${testapp}_test C CXX)
    add_definitions(-DTESTING_DIR_${testapp}=${CMAKE_CURRENT_SOURCE_DIR}/tests/${testapp})
    set(${testapp}_DIR ${testapp})

    file(GLOB ${testapp}_SRC
      "${${testapp}_DIR}/*.h"
      "${${testapp}_DIR}/*.cpp"
    )

    add_executable(${testapp}_test ${${testapp}_SRC})
    set_target_properties(${testapp}_test PROPERTIES LINKER_LANGUAGE CXX)

    file(GLOB ${testapp}_PROTO
      "${${testapp}_DIR}/*.proto"
    )

    if (NOT ${testapp}_PROTO STREQUAL "")
      SET(PROTOBUF_GENERATE_CPP_APPEND_PATH ${${testapp}_DIR})
      protobuf_generate_cpp(${testapp}_PROTO_SRC ${testapp}_PROTO_HDR
        ${${testapp}_PROTO}
      )
      add_library(${testapp}_PROTO_LIB SHARED ${${testapp}_PROTO_HDR} ${${testapp}_PROTO_SRC})

      include_directories(${CMAKE_CURRENT_BINARY_DIR})
      target_link_libraries(${testapp}_test ${testapp}_PROTO_LIB ${PROTOBUF_LIBRARY})
    endif(NOT ${testapp}_PROTO STREQUAL "")
endmacro()


define_testapp(banned_domains_sanitize)
define_testapp(personal_budget_sanitize)
define_testapp(transport_E_sanitize)
define_testapp(sum_no_overflow)
define_testapp(transport)
define_testapp(svg)
define_testapp(json_printer)
define_testapp(bin_saveload)
define_testapp(phone_book)
define_testapp(yellow_pages)
define_testapp(nucleotide)
define_testapp(mython)
define_testapp(optional)
define_testapp(equals_to_one_of)

