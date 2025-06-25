cmake_minimum_required(VERSION 3.10)
project(mimic99-stage12)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

# Include directory
include_directories(include)

# Glob all source files in src/
file(GLOB SRC_FILES src/*.c)
set(MAIN_SRC src/main.c)

# Create a library from all source files except main.c
list(REMOVE_ITEM SRC_FILES ${MAIN_SRC})
add_library(mimic99_core STATIC ${SRC_FILES})

# ---- Main executable ----
add_executable(mimic99 ${SRC_FILES} ${MAIN_SRC})
target_include_directories(mimic99 PRIVATE include)

# ---- Unit tests ----

# Glob all test_*.c files
file(GLOB TEST_FILES unit_tests/test_*.c)

foreach(TEST_FILE ${TEST_FILES})
    # Extract just the filename without path or extension
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    # Add executable for each test
    add_executable(${TEST_NAME} ${TEST_FILE})

    # Link with core library
    target_link_libraries(${TEST_NAME} mimic99_core)

    # Include headers
    target_include_directories(${TEST_NAME} PRIVATE include)

    # Put output in unit_tests/build/
    set_target_properties(${TEST_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/unit_tests/build
    )
endforeach()

add_custom_target(run_unit_tests
        COMMAND ./run_unit_tests.sh
        DEPENDS ${TEST_FILES}
        COMMENT "Running unit tests via script")

add_custom_target(run_all_tests
        COMMAND ./run_all_tests.sh mimic99
        DEPENDS mimic99
        COMMENT "Running all tests via script")
