# © Joseph Cameron - MIT License

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

if (TARGET gdk::timing)
    return()
endif()

project(gdk-timing
    VERSION 1.0
    DESCRIPTION "a game loop: frames at a capped rate, a fixed-step simulation beside them, a time scale"
    LANGUAGES CXX)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/jfc-cmake/jfclib.cmake")

option(GDKTIMING_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})
option(GDKTIMING_BUILD_DOCS "Build documentation" OFF)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/permissive->)
else()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
    endif()

    include(CheckCXXCompilerFlag)

    foreach (_strict_flag "-Werror=c++20-extensions" "-Werror=c++23-extensions")
        string(MAKE_C_IDENTIFIER "HAS${_strict_flag}" _strict_flag_symbol)

        check_cxx_compiler_flag("${_strict_flag}" ${_strict_flag_symbol})

        if (${_strict_flag_symbol})
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${_strict_flag}>)
        endif()
    endforeach()
endif()

include(GNUInstallDirs)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(_gdktiming_sleep "windows")
else()
    set(_gdktiming_sleep "std")
endif()

add_library(gdktiming STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game_loop.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/${_gdktiming_sleep}_sleep.cpp")

add_library(gdk::timing ALIAS gdktiming)

add_library(gdk::time ALIAS gdktiming)
add_library(gdktime ALIAS gdktiming)

set_target_properties(gdktiming PROPERTIES EXPORT_NAME timing PREFIX "lib")

target_include_directories(gdktiming
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/include")

target_compile_features(gdktiming PUBLIC cxx_std_20)

include(CheckCXXSourceCompiles)

function(_gdktiming_link_atomic_if_needed aTarget)
    set(CMAKE_CXX_STANDARD 20)

    set(_source "
#include <atomic>
int main() {
    std::atomic<double> a{0};
    a.store(1, std::memory_order_relaxed);
    return static_cast<int>(a.load(std::memory_order_relaxed));
}")

    check_cxx_source_compiles("${_source}" GDKTIMING_ATOMIC_DOUBLE_WITHOUT_LIBATOMIC)

    if (GDKTIMING_ATOMIC_DOUBLE_WITHOUT_LIBATOMIC)
        return()
    endif()

    set(CMAKE_REQUIRED_LIBRARIES atomic)

    check_cxx_source_compiles("${_source}" GDKTIMING_ATOMIC_DOUBLE_WITH_LIBATOMIC)

    if (NOT GDKTIMING_ATOMIC_DOUBLE_WITH_LIBATOMIC)
        message(FATAL_ERROR "gdk-timing: std::atomic<double> does not link, with or without libatomic")
    endif()

    target_link_libraries(${aTarget} PUBLIC atomic)
endfunction()

_gdktiming_link_atomic_if_needed(gdktiming)

include(CMakePackageConfigHelpers)

install(TARGETS gdktiming
    EXPORT gdk-timing-targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(EXPORT gdk-timing-targets
    FILE gdk-timing-targets.cmake
    NAMESPACE gdk::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gdk-timing")

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/gdk-timing-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gdk-timing-config.cmake"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/gdk-timing-targets.cmake\")
")

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/gdk-timing-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/gdk-timing-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gdk-timing")

if (GDKTIMING_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (GDKTIMING_BUILD_DOCS)
    add_subdirectory(docs)
endif()
