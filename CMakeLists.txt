cmake_minimum_required(VERSION 3.12)

project(red_belt)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

###########################################
## UPDATE_FIELD
###########################################

set(UPDATE_FIELD_DIR UPDATE_FIELD)

set(UPDATE_FIELD_SRC
    ${UPDATE_FIELD_DIR}/update_field_main.cpp
)

add_executable(UPDATE_FIELD_test ${UPDATE_FIELD_SRC})

###########################################
## bad_macro
###########################################

set(bad_macro_DIR bad_macro)

set(bad_macro_SRC
    ${bad_macro_DIR}/bad_macro_main.cpp
)

add_executable(bad_macro_test ${bad_macro_SRC})

###########################################
## UNIQ_ID
###########################################

set(UNIQ_ID_DIR UNIQ_ID)

set(UNIQ_ID_SRC
    ${UNIQ_ID_DIR}/uniq_id_main.cpp
)

add_executable(uniq_id_test ${UNIQ_ID_SRC})

###########################################
## table
###########################################

set(table_DIR table)

set(table_SRC
    ${table_DIR}/table_main.cpp
)

add_executable(table_test ${table_SRC})

###########################################
## deque_via_two_vectors
###########################################

set(deque_via_two_vectors_DIR deque_via_two_vectors)

set(deque_via_two_vectors_SRC
    ${deque_via_two_vectors_DIR}/deque_via_two_vectors_main.cpp
)

add_executable(deque_via_two_vectors_test ${deque_via_two_vectors_SRC})

###########################################
## paginator
###########################################

set(paginator_DIR paginator)

set(paginator_SRC
    ${paginator_DIR}/paginator_main.cpp
)

add_executable(paginator_test ${paginator_SRC})

###########################################
## g
###########################################

set(sort_students_DIR sort_students)

set(sort_students_SRC
    ${sort_students_DIR}/sort_students_main.cpp
)

add_executable(sort_students_test ${sort_students_SRC})

###########################################
## learner
###########################################

set(learner_DIR learner)

set(learner_SRC
    ${learner_DIR}/learner_main.cpp
)

add_executable(learner_test ${learner_SRC})

###########################################
## expresses
###########################################

set(expresses_DIR expresses)

set(expresses_SRC
    ${expresses_DIR}/expresses_main.cpp
)

add_executable(expresses_test ${expresses_SRC})

###########################################
## expresses
###########################################

set(ebook_DIR ebook)

set(ebook_SRC
    ${ebook_DIR}/ebook.cpp
)

add_executable(ebook_test ${ebook_SRC})

