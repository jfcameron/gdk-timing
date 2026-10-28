# © Joseph Cameron - MIT License

find_package(Threads REQUIRED)

jfc_add_tests(
    C++_STANDARD 20

    TEST_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/game_loop_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/fixed_step_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rate_types_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/host_loop_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ticked_frame_time_test.cpp"

    LIBRARIES
        gdktiming
        Threads::Threads)

option(GDKTIMING_TEST_PACING_FLOOR
    "Test that a capped loop keeps close to its cap; off where the OS throttles sleeps, as on macOS CI runners" ON)

target_compile_definitions(${JFC_TEST_TARGET} PRIVATE
    GDKTIMING_TEST_PACING_FLOOR=$<BOOL:${GDKTIMING_TEST_PACING_FLOOR}>)

file(GLOB_RECURSE GDKTIMING_PUBLIC_HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/../include/gdk/*.h")

set(GDKTIMING_SELF_CONTAINMENT_SOURCES "")

foreach (_header ${GDKTIMING_PUBLIC_HEADERS})
    file(RELATIVE_PATH _relative "${CMAKE_CURRENT_LIST_DIR}/../include" "${_header}")

    string(MAKE_C_IDENTIFIER "${_relative}" _symbol)

    set(_generated "${CMAKE_CURRENT_BINARY_DIR}/self_containment/${_symbol}.cpp")

    file(WRITE "${_generated}" "#include <${_relative}>\n")

    list(APPEND GDKTIMING_SELF_CONTAINMENT_SOURCES "${_generated}")
endforeach()

add_library(gdktiming_header_self_containment OBJECT ${GDKTIMING_SELF_CONTAINMENT_SOURCES})

target_link_libraries(gdktiming_header_self_containment PRIVATE gdktiming)

set_target_properties(gdktiming_header_self_containment PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

if (MSVC)
    target_compile_options(gdktiming_header_self_containment PRIVATE /permissive- /W4 /WX)
else()
    target_compile_options(gdktiming_header_self_containment PRIVATE
        -pedantic-errors -Wall -Wextra -Werror)
endif()

if (NOT CMAKE_CROSSCOMPILING)
    add_test(NAME gdk-timing_install_and_consume
        COMMAND "${CMAKE_COMMAND}"
            "-DBUILD_DIR=${PROJECT_BINARY_DIR}"
            "-DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/install_and_consume"
            "-DCONFIG=$<CONFIG>"
            "-DGENERATOR=${CMAKE_GENERATOR}"
            "-DPLATFORM=${CMAKE_GENERATOR_PLATFORM}"
            "-DTOOLSET=${CMAKE_GENERATOR_TOOLSET}"
            "-DCXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-DCXX_FLAGS=${CMAKE_CXX_FLAGS}"
            "-DEXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}"
            -P "${CMAKE_CURRENT_LIST_DIR}/install/install_and_consume.cmake")
endif()
