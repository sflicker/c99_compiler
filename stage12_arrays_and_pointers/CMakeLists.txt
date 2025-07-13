cmake_minimum_required(VERSION 3.10...4.0)
project(
        mimic99-stage12
        VERSION 1.0
        LANGUAGES C ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

# Include directory
include_directories(include)

# Glob all source files in src/
file(GLOB SRC_FILES src/*.c)
set(MAIN_SRC src/main.c
)
list(REMOVE_DUPLICATES SRC_FILES)

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

# ---- Build all .asm files in asm/ ----

# Find all .asm files in asm/ directory
file(GLOB ASM_FILES ${CMAKE_SOURCE_DIR}/asm/*.asm)

foreach(ASM_FILE ${ASM_FILES})
    get_filename_component(ASM_NAME ${ASM_FILE} NAME_WE)

    # Define the object file name in the build directory
    set(ASM_OBJ ${CMAKE_BINARY_DIR}/${ASM_NAME}.o)

    # Custom command to assemble .asm → .o
    add_custom_command(
            OUTPUT ${ASM_OBJ}
            COMMAND nasm -f elf64 -g -F dwarf -o ${ASM_OBJ} ${ASM_FILE}
            DEPENDS ${ASM_FILE}
            COMMENT "Assembling ${ASM_NAME}.asm → ${ASM_NAME}.o with debug info"
    )

    # Executable target for each .asm file
    add_executable(${ASM_NAME} ${ASM_OBJ})

    # Tell CMake how to link it
    set_target_properties(${ASM_NAME} PROPERTIES LINKER_LANGUAGE C)

    # Optional: put all asm executables into asm/build/
    set_target_properties(${ASM_NAME} PROPERTIES
            LINKER_LANGUAGE C
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/asm/build
            LINK_FLAGS "-nostartfiles -nostdlib"
    )
endforeach()