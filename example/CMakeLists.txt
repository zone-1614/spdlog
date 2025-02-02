# Copyright(c) 2019 spdlog authors Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.10)
project(spdlog_examples CXX)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

# ---------------------------------------------------------------------------------------
# Example of using pre-compiled library
# ---------------------------------------------------------------------------------------
add_executable(example example.cpp)
target_link_libraries(example PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)

# ---------------------------------------------------------------------------------------
# Example of using header-only library
# ---------------------------------------------------------------------------------------
if(SPDLOG_BUILD_EXAMPLE_HO)
    add_executable(example_header_only example.cpp)
    target_link_libraries(example_header_only PRIVATE spdlog::spdlog_header_only)
endif()

add_executable(steal_str steal_str.cpp)
target_link_libraries(steal_str PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)

if(SPDLOG_BUILD_EXAMPLE_HO)
    add_executable(steal_str_header_only steal_str.cpp)
    target_link_libraries(steal_str_header_only PRIVATE spdlog::spdlog_header_only)
endif()

add_executable(steal_str2 steal_str2.cpp)
target_link_libraries(steal_str2 PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)

if(SPDLOG_BUILD_EXAMPLE_HO)
    add_executable(steal_str2_header_only steal_str2.cpp)
    target_link_libraries(steal_str2_header_only PRIVATE spdlog::spdlog_header_only)
endif()