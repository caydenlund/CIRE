cmake_minimum_required(VERSION 3.22)
project(CIRE)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_subdirectory(lib)
add_subdirectory(benchmarks)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    include_directories(include
            /usr/local/include
            /usr/local/include/ibex
            /usr/local/include/ibex/3rd
            /usr/local/include/ibex/3rd/gaol)
    link_directories(/usr/local/lib)
else()
    include_directories(include
            /usr/local/include/ibex
            /usr/local/include/ibex/3rd
            /usr/local/include/ibex/3rd/gaol)
endif()

option(ENABLE_LLVM_FRONTEND "Enable the LLVM frontend" OFF)

if(ENABLE_LLVM_FRONTEND)
#===============================================================================
# 1. LOAD LLVM CONFIGURATION
#===============================================================================
# Set this to a valid LLVM installation dir
set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")
message(STATUS "LLVM installation directory: ${LT_LLVM_INSTALL_DIR}")

# Add the location of LLVMConfig.cmake to CMake search paths (so that
# find_package can locate it)
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

find_package(LLVM CONFIG)
if("${LLVM_VERSION_MAJOR}" VERSION_LESS 16)
    message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 16 or above")
endif()

# HelloWorld includes headers from LLVM - update the include paths accordingly
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

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
add_library(CIRE_LLVM SHARED
            src/CIRE_LLVM.cpp
            src/CIRE.cpp)
target_link_libraries(CIRE_LLVM
                        PUBLIC Graph ibex
                        PRIVATE nlohmann_json::nlohmann_json)

# Allow undefined symbols in shared objects on Darwin (this is the default
# behaviour on Linux)
#target_link_libraries(HelloWorld
#        "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")

#===============================================================================
ENDIF(ENABLE_LLVM_FRONTEND)

# Building lib_tester
add_executable(lib_tester src/lib_tester.cpp)
target_link_libraries(lib_tester
                        PUBLIC Graph ibex
                        PRIVATE nlohmann_json::nlohmann_json)

# Building CIRE
add_executable(CIRE src/CIRE.cpp src/Results.cpp)
target_link_libraries(CIRE
                        PUBLIC Graph ibex
                        PRIVATE nlohmann_json::nlohmann_json)
