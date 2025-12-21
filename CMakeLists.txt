cmake_minimum_required(VERSION 3.15)
project(XeryonLib VERSION 0.1 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option: build shared library instead of static
option(BUILD_SHARED_LIB "Build XeryonLib as a shared library" OFF)

# Option: build example executable using the library
option(BUILD_EXAMPLES "Build example executable my_app" OFF)

# Use new Boost finding policy (suppresses the warning)
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# Find dependencies (Boost.System is used by Boost.Asio)
find_package(Threads REQUIRED)
include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG master
)
FetchContent_MakeAvailable(asio)

# Collect sources (implementation files only; headers are installed via include/)
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.cxx" "src/*.cc")

# Create library target
if(BUILD_SHARED_LIB)
    add_library(XeryonLib SHARED ${LIB_SOURCES})
else()
    add_library(XeryonLib STATIC ${LIB_SOURCES})
endif()

# Public include directory
target_include_directories(XeryonLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(XeryonLib
    PRIVATE
        Threads::Threads
)

# Include standalone Asio headers and define ASIO_STANDALONE so code uses asio:: namespace
target_include_directories(XeryonLib PRIVATE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(XeryonLib PRIVATE ASIO_STANDALONE)

# Compiler warnings
target_compile_options(XeryonLib PRIVATE -Wall -Wextra -pedantic)

## Sanitizers (optional)
option(ENABLE_ASAN "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR ENABLE_ASAN)
    message(STATUS "Building with sanitizers/debug flags enabled")
    target_compile_options(XeryonLib PRIVATE -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(XeryonLib PRIVATE -fsanitize=address,undefined)
endif()

# Example executable (optional) — links against the library
if(BUILD_EXAMPLES)
    add_executable(my_app src/main.cpp)
    target_link_libraries(my_app PRIVATE XeryonLib)
    target_include_directories(my_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set_target_properties(my_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# Notes on asio/Boost.Asio handling
message(STATUS "Using standalone Asio fetched by FetchContent")

# If you want to avoid requiring a system Boost installation you have a few options:
# 1) Use the standalone Asio (https://github.com/chriskohlhoff/asio) and adjust includes to
#    `#include <asio.hpp>` and define ASIO_STANDALONE. You can vendor it with FetchContent.
# 2) Vendor Boost into the project (heavy) or use a package manager (conan/vcpkg) to provide Boost.
# 3) Keep FindBoost (as used) and require users to have Boost available (simplest).

# Example (commented) to pull standalone asio with FetchContent (requires header change to <asio.hpp>):
# include(FetchContent)
# FetchContent_Declare(asio URL https://github.com/chriskohlhoff/asio/archive/refs/heads/master.zip)
# FetchContent_MakeAvailable(asio)
# target_include_directories(XeryonLib PRIVATE ${asio_SOURCE_DIR}/asio/include)
# target_compile_definitions(XeryonLib PRIVATE ASIO_STANDALONE)

# Install rules (optional)
include(GNUInstallDirs)
install(TARGETS XeryonLib EXPORT XeryonLibTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT XeryonLibTargets FILE XeryonLibTargets.cmake NAMESPACE Xeryon:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XeryonLib)
