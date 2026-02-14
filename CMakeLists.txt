cmake_minimum_required(VERSION 3.22)
project(CIRE)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
                    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(json)


# Building Scanner and Parser
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

add_subdirectory(benchmarks)

set(IBEX_INSTALL_DIR "/home/cayden/git/cire/ibex-2.8.9-install" CACHE PATH "Ibex installation directory")

include_directories(include
    include/cire
    ${CMAKE_CURRENT_BINARY_DIR}
    ${IBEX_INSTALL_DIR}/include
    ${IBEX_INSTALL_DIR}/include/ibex
    ${IBEX_INSTALL_DIR}/include/ibex/3rd)

link_directories(${IBEX_INSTALL_DIR}/lib
    ${IBEX_INSTALL_DIR}/lib/ibex/3rd)

BISON_TARGET(MyParser
        src/frontend/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h)
FLEX_TARGET(MyScanner
        src/frontend/lexer.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

add_library(Graph STATIC
        src/interfaces/Logging.cpp
        src/core/Graph.cpp
        src/core/Node.cpp
        src/core/SymbolTable.cpp
        src/core/ErrorAnalyzer.cpp
        src/interfaces/IBEXInterface.cpp
        ${BISON_MyParser_OUTPUTS}
        ${FLEX_MyScanner_OUTPUTS})

target_link_libraries(Graph PUBLIC nlohmann_json::nlohmann_json)

option(ENABLE_LLVM_FRONTEND "Enable the LLVM frontend" OFF)
option(LLVM_STATIC_LINKING "Link LLVM libraries statically" OFF)

if(ENABLE_LLVM_FRONTEND)
    # Add LLVM-dependent sources to Graph library
    target_sources(Graph PRIVATE
        src/core/InstructionMetadata.cpp
        src/core/SourceMapper.cpp)
    #===============================================================================
    # 1. LOAD LLVM CONFIGURATION
    #===============================================================================
    # Set this to a valid LLVM installation dir
    set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")
    message(STATUS "LLVM installation directory: ${LT_LLVM_INSTALL_DIR}")

    # Add the location of LLVMConfig.cmake to CMake search paths (so that
    # find_package can locate it)
    list(PREPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

    find_package(LLVM REQUIRED CONFIG)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    if("${LLVM_VERSION_MAJOR}" VERSION_LESS 15)
        message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 16 or above")
    endif()

    # Configure static linking if requested
    if(LLVM_STATIC_LINKING)
        message(STATUS "Configuring for static LLVM linking")
        set(LLVM_LINK_LLVM_DYLIB OFF)
        set(BUILD_SHARED_LIBS OFF)

        # Add system libraries needed when statically linking LLVM
        find_package(Threads REQUIRED)

        # Try to find static versions of system libraries, fall back to shared if needed
        set(_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a .so)

        find_library(ZLIB_LIB NAMES z libz)
        find_library(CURSES_LIB NAMES ncurses tinfo)

        # Restore original library suffixes
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})

        set(LLVM_SYSTEM_LIBS
            ${CMAKE_THREAD_LIBS_INIT}
            ${CMAKE_DL_LIBS}
            m
        )

        # Add optional system libraries if found
        if(ZLIB_LIB)
            list(APPEND LLVM_SYSTEM_LIBS ${ZLIB_LIB})
            message(STATUS "Found zlib: ${ZLIB_LIB}")
        endif()

        if(CURSES_LIB)
            list(APPEND LLVM_SYSTEM_LIBS ${CURSES_LIB})
            message(STATUS "Found curses: ${CURSES_LIB}")
        endif()

        message(STATUS "LLVM static linking enabled - binary will be self-contained")
    else()
        message(STATUS "Using dynamic LLVM linking")
        set(LLVM_SYSTEM_LIBS "")
    endif()

    # Add the AddLLVM CMake module
    include(AddLLVM)
    # Update the include paths to include header directories from LLVM
    include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

    set(LLVM_UTIL_SRCS
            "src/utils/llvm_utils.cpp"
    )

    set(UTIL_SRCS
            "src/frontend/llvm_frontend.cpp"
    )

    add_library(llvm_util STATIC ${LLVM_UTIL_SRCS})
    add_library(util STATIC ${UTIL_SRCS})

    # util depends on Graph for SourceMapper and other Graph components
    target_link_libraries(util PUBLIC Graph)

    # Link LLVM libraries to Graph for LLVM-dependent sources
    llvm_map_components_to_libnames(graph_llvm_libs support core irreader)
    target_link_libraries(Graph PUBLIC ${graph_llvm_libs} ${LLVM_SYSTEM_LIBS})

    #===============================================================================
    # 2. LLVM-TUTOR BUILD CONFIGURATION
    #===============================================================================
    # LLVM is normally built without RTTI. Be consistent with that.
    if(NOT LLVM_ENABLE_RTTI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    endif()

    #===============================================================================
    # 3. ADD THE TARGET
    #===============================================================================
    add_library(cire STATIC
            src/bin/cire_pass.cpp
            src/core/Cire.cpp
            src/core/Results.cpp)
    target_link_libraries(cire
                            PUBLIC Graph ibex gmp z gaol soplex ultim gdtoa
                            PRIVATE nlohmann_json::nlohmann_json llvm_util util)

    add_llvm_executable(CIRE_LLVM src/bin/CIRE_LLVM.cpp src/core/Cire.cpp src/core/Results.cpp)
    llvm_map_components_to_libnames(llvm_libs support core irreader transformutils)
    target_link_libraries(CIRE_LLVM
            PUBLIC Graph ibex gmp z gaol soplex ultim gdtoa
            PRIVATE nlohmann_json::nlohmann_json llvm_util util ${llvm_libs} ${LLVM_SYSTEM_LIBS})

    # Building VALIDATOR
    add_executable(VALIDATOR src/bin/VALIDATOR.cpp)
    target_link_libraries(VALIDATOR
                            PRIVATE llvm_util util ${llvm_libs} ${LLVM_SYSTEM_LIBS})

    # Allow undefined symbols in shared objects on Darwin (this is the default
    # behaviour on Linux)
    #target_link_libraries(HelloWorld
    #        "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")

    #===============================================================================
ENDIF(ENABLE_LLVM_FRONTEND)

# Building lib_tester
add_executable(lib_tester src/bin/lib_tester.cpp)
target_link_libraries(lib_tester
                        PUBLIC Graph ibex gmp z gaol soplex ultim gdtoa
                        PRIVATE nlohmann_json::nlohmann_json)

# Building CIRE
add_executable(CIRE src/bin/CIRE.cpp src/core/Cire.cpp src/core/Results.cpp)
target_link_libraries(CIRE
                        PUBLIC Graph ibex gmp z gaol soplex ultim gdtoa
                        PRIVATE nlohmann_json::nlohmann_json)

add_custom_target("benchmark"
                   COMMAND "cd scripts"
                   COMMAND "python3"
                           "${PROJECT_SOURCE_DIR}/scripts/benchmark_c.py"
                           "${PROJECT_SOURCE_DIR}/benchmarks/fpbench_ll"
                    DEPENDS CIRE_LLVM
                    USES_TERMINAL)

add_custom_target("check"
                   COMMAND "python3"
                           "${PROJECT_SOURCE_DIR}/scripts/lit/lit.py"
                           "${PROJECT_SOURCE_DIR}/benchmarks"
                   DEPENDS CIRE_LLVM
                   USES_TERMINAL
                 )
