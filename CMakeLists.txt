cmake_minimum_required(VERSION 3.0)

function(get_litelsm_version version_var)
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/include/litelsm/version.h" version_header_file)
  foreach(component MAJOR MINOR PATCH)
    string(REGEX MATCH "#define LITELSM_${component} ([0-9]+)" _ ${version_header_file})
    set(LITELSM_VERSION_${component} ${CMAKE_MATCH_1})
  endforeach()
  set(${version_var} "${LITELSM_VERSION_MAJOR}.${LITELSM_VERSION_MINOR}.${LITELSM_VERSION_PATCH}" PARENT_SCOPE)
endfunction()

get_litelsm_version(litelsm_VERSION)
project(litelsm
  VERSION ${litelsm_VERSION}
  DESCRIPTION "A key-value storage engine that utilizes the LSM data structure"
  LANGUAGES CXX )

set(CMAKE_CXX_STANDARD 17)

include(GoogleTest)

find_package(GTest REQUIRED)

set(OS_INCLUDE_PATH /usr/include /usr/local/include)
include_directories(${OS_INCLUDE_PATH})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
option(WITH_TESTS "build with tests" ON)

set(LITELSM_STATIC_LIB litelsm)

set(SOURCES
    common/coding.cpp
    common/comparator.cpp
    filesystem/posix_filesystem.cpp
    filesystem/posix_file.cpp
    filesystem/io_error.cpp
    util/uuid_gen.cpp
    util/crc32c.cpp
    util/hash.cpp
    util/bloom.cpp
    util/string_util.cpp
    storage/data_page_builder.cpp
    storage/data_page_reader.cpp
    storage/filter_page.cpp
    )

set(SYSTEM_LIBS ${CMAKE_THREAD_LIBS_INIT})
  
add_library(${LITELSM_STATIC_LIB} STATIC ${SOURCES})
target_link_libraries(${LITELSM_STATIC_LIB} PRIVATE
  ${SYSTEM_LIBS})

set(LITELSM_LIB ${LITELSM_STATIC_LIB})

if(WITH_TESTS)
    list(APPEND TESTS
        util/slice_test.cpp
        util/status_test.cpp
        util/uuid_gen_test.cpp
        util/defer_op_test.cpp
        util/crc32c_test.cpp
        util/hash_test.cpp
        util/bloom_test.cpp
        filesystem/filesystem_test.cpp
        filesystem/posix_file_test.cpp
        storage/data_page_test.cpp
        storage/filter_page_test.cpp
    )
    message(STATUS "TESTS: ${TESTS}")
    foreach(sourcefile ${TESTS})
        get_filename_component(testname ${sourcefile} NAME_WE)
        add_executable(${testname} ${sourcefile})
        target_link_libraries(${testname} ${LITELSM_LIB} ${GTEST_BOTH_LIBRARIES})
    endforeach()
endif()
