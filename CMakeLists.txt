cmake_minimum_required(VERSION 3.24)

project(v8unpack VERSION 3.1.0.0 LANGUAGES CXX)

# Keep the fourth component explicit for Windows VERSIONINFO and release tags.
set(V8UNPACK_WINDOWS_VERSION_TWEAK 0)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json" V8UNPACK_VCPKG_MANIFEST)
string(JSON V8UNPACK_MANIFEST_VERSION GET "${V8UNPACK_VCPKG_MANIFEST}" version-string)
if(NOT V8UNPACK_MANIFEST_VERSION STREQUAL PROJECT_VERSION)
    message(FATAL_ERROR
        "Version mismatch: CMake=${PROJECT_VERSION}, vcpkg=${V8UNPACK_MANIFEST_VERSION}")
endif()

include(CTest)

option(V8UNPACK_STATIC_RUNTIME "Link the MSVC runtime statically" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ZLIB REQUIRED)

configure_file(cmake/version.h.in generated/version.h @ONLY)
if(WIN32)
    configure_file(cmake/VersionInfo.rc.in generated/VersionInfo.rc @ONLY)
endif()

add_library(v8unpack_core STATIC
    src/V8File.cpp
    src/V8File.h
    src/VersionFile.cpp
    src/VersionFile.h
    src/placeholder216.cpp
    src/utils.cpp
    src/filesystem_compat.h
)

target_include_directories(v8unpack_core PUBLIC
    src
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)
target_compile_definitions(v8unpack_core
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
target_compile_options(v8unpack_core PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
target_link_libraries(v8unpack_core
    PUBLIC ZLIB::ZLIB
)

add_executable(v8unpack src/main.cpp)
target_link_libraries(v8unpack PRIVATE v8unpack_core)
target_compile_options(v8unpack PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

if(WIN32)
    target_sources(v8unpack PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated/VersionInfo.rc")
endif()

if(MSVC AND V8UNPACK_STATIC_RUNTIME)
    set_property(TARGET v8unpack_core v8unpack PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

install(TARGETS v8unpack RUNTIME DESTINATION bin)
install(FILES README.md LICENSE DESTINATION share/doc/v8unpack)

if(BUILD_TESTING)
    add_test(
        NAME integration_roundtrip
        COMMAND ${CMAKE_COMMAND}
            -DPROGRAM=$<TARGET_FILE:v8unpack>
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/test-work/integration
            -P ${CMAKE_CURRENT_SOURCE_DIR}/test/integration_roundtrip.cmake
    )

    add_test(
        NAME cli_smoke
        COMMAND ${CMAKE_COMMAND}
            -DPROGRAM=$<TARGET_FILE:v8unpack>
            -DEXPECTED_VERSION=${PROJECT_VERSION}
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/test-work/cli
            -P ${CMAKE_CURRENT_SOURCE_DIR}/test/cli_smoke.cmake
    )

    set(LOCAL_EPF_FIXTURE
        "${CMAKE_CURRENT_SOURCE_DIR}/test/local-fixtures/cjk-evotor-settings-util.epf")
    if(EXISTS "${LOCAL_EPF_FIXTURE}")
        add_test(
            NAME local_epf_golden_roundtrip
            COMMAND ${CMAKE_COMMAND}
                -DPROGRAM=$<TARGET_FILE:v8unpack>
                -DFIXTURE=${LOCAL_EPF_FIXTURE}
                -DEXPECTED_SHA256=FA306835F54C01FA59A3C3B244F653A0E9CA7F1484BA6FE2035185502C5174D7
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/test-work/local-epf
                -P ${CMAKE_CURRENT_SOURCE_DIR}/test/local_fixture_roundtrip.cmake
        )
    endif()
endif()
