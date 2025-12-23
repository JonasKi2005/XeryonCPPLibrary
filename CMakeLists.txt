cmake_minimum_required(VERSION 3.15)
project(XeryonLib VERSION 0.1 LANGUAGES CXX)

# --- C++ Standard ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Options ---
option(BUILD_SHARED_LIB "Build XeryonLib as a shared library" OFF)
option(BUILD_EXAMPLES "Build example executable my_app" OFF)

# --- Dependencies ---
# 1. Threads (Required by Asio)
find_package(Threads REQUIRED)

# 2. Fetch Asio (Standalone)
include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG master
)
FetchContent_MakeAvailable(asio)
message(STATUS "Using standalone Asio fetched via FetchContent")

# --- Source Management ---
# Collect all sources
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.cxx" "src/*.cc")

# IMPORTANT: Remove main.cpp from the library sources. 
# We only want main.cpp in the executable, not the library itself.
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*src/main\\.cpp$")

# --- Create Library Target ---
if(BUILD_SHARED_LIB)
    add_library(XeryonLib SHARED ${LIB_SOURCES})
else()
    add_library(XeryonLib STATIC ${LIB_SOURCES})
endif()

# --- Include Directories ---
target_include_directories(XeryonLib
    PUBLIC 
        # Use $<BUILD_INTERFACE:...> so Asio is only linked during the build phase
        $<BUILD_INTERFACE:${asio_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# --- Compiler Definitions ---
# PUBLIC so consumers also know we are using standalone mode
target_compile_definitions(XeryonLib PUBLIC ASIO_STANDALONE)

# --- Link Dependencies ---
target_link_libraries(XeryonLib
    PRIVATE
        Threads::Threads
)

# --- Compiler Warnings ---
if (MSVC)
    target_compile_definitions(XeryonLib PRIVATE _USE_MATH_DEFINES)
    target_compile_options(XeryonLib PRIVATE /W4 /wd4820)
else ()
    target_compile_options(XeryonLib PRIVATE -Wall -Wextra -pedantic)
endif()

# --- Sanitizers (Debug Helper) ---
option(ENABLE_ASAN "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR ENABLE_ASAN)
    message(STATUS "Building with sanitizers/debug flags enabled")
    target_compile_options(XeryonLib PRIVATE -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(XeryonLib PRIVATE -fsanitize=address,undefined)
endif()

# --- Example Executable ---
if(BUILD_EXAMPLES)
    add_executable(my_app src/main.cpp)
    
    # Linking XeryonLib automatically pulls in the Asio include paths 
    # because we marked them as PUBLIC above.
    target_link_libraries(my_app PRIVATE XeryonLib)
    
    set_target_properties(my_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# --- Install Rules ---
include(GNUInstallDirs)
install(TARGETS XeryonLib EXPORT XeryonLibTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT XeryonLibTargets 
    FILE XeryonLibTargets.cmake 
    NAMESPACE Xeryon:: 
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XeryonLib
)