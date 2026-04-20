cmake_minimum_required(VERSION 3.15)
project(fastpurge VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ── Sources ────────────────────────────────────────────────────────────────
add_executable(fastpurge
	src/main.c
	src/args.c
	src/scan.c
	src/worker.c
	src/ui.c
)

# ── Includes ───────────────────────────────────────────────────────────────
target_include_directories(fastpurge PRIVATE include)

# ── Compiler warnings ──────────────────────────────────────────────────────
target_compile_options(fastpurge PRIVATE
	-Wall -Wextra -Wpedantic -Wshadow
	-Wformat=2 -Wconversion -Wsign-conversion
	-Wstrict-prototypes -Wmissing-prototypes
)

# ── Sanitizers on Debug ────────────────────────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_options(fastpurge PRIVATE -fsanitize=address,undefined)
	target_link_options(fastpurge    PRIVATE -fsanitize=address,undefined)
endif()

# ── Release: optimize + strip ──────────────────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_options(fastpurge PRIVATE -O2)
	if(NOT APPLE)
		target_link_options(fastpurge PRIVATE -s -static-libgcc)
	endif()
endif()

install(TARGETS fastpurge RUNTIME DESTINATION bin)

message(STATUS "fastpurge ${PROJECT_VERSION}  |  ${CMAKE_SYSTEM_NAME}  |  ${CMAKE_BUILD_TYPE}")
