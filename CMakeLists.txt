cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(RackspaceBackup)
set(CMAKE_CXX_STANDARD 17)

find_package(fmt CONFIG REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GIT_REV "N/A")
    set(GIT_DIFF "")
    set(GIT_TAG "N/A")
    set(GIT_BRANCH "N/A")

    configure_file(
      "${PROJECT_SOURCE_DIR}/git_hash.h.in"
      "${CMAKE_CURRENT_BINARY_DIR}/git_hash.h"
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_target(gen-version-h
      COMMAND "${CMAKE_COMMAND}"
      "-D" "SRC_DIR=${PROJECT_SOURCE_DIR}"
      "-D" "BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}"
      "-P" "${PROJECT_SOURCE_DIR}/version.cmake"
      COMMENT "Generating git version file"
    )
endif()

add_executable(rackspacebackup main.cpp)

target_link_libraries(rackspacebackup PRIVATE fmt::fmt-header-only)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_dependencies(rackspacebackup gen-version-h)
endif()


############################
# Copy to source directory
add_custom_target(
    copy-compile-commands ALL
    DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json
    )
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json
    DEPENDS
        # Unlike "proper" targets like executables and libraries, 
        # custom command / target pairs will not set up source
        # file dependencies, so we need to list file explicitly here
        generate-compile-commands
        ${CMAKE_BINARY_DIR}/compile_commands.json
    )

# Generate the compilation commands. Necessary so cmake knows where it came
# from and if for some reason you delete it.
add_custom_target(generate-compile-commands
    DEPENDS
        ${CMAKE_BINARY_DIR}/compile_commands.json
    )
############################