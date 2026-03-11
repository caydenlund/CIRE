cmake_minimum_required(VERSION 3.20)
project(cire CXX)
set(CMAKE_CXX_STANDARD 17)

option(CIRE_ENABLE_LLVM_FRONTEND "Build CIRE_LLVM (requires LLVM)" OFF)

# ---------------------------------------------------------------
# IBEX configuration
# ---------------------------------------------------------------
set(IBEX_INSTALL_DIR "/home/cayden/git/cire/ibex-2.8.9-install" CACHE PATH "IBEX installation directory")

include_directories(
    ${IBEX_INSTALL_DIR}/include
    ${IBEX_INSTALL_DIR}/include/ibex
    ${IBEX_INSTALL_DIR}/include/ibex/3rd)

link_directories(
    ${IBEX_INSTALL_DIR}/lib
    ${IBEX_INSTALL_DIR}/lib/ibex/3rd)

include(FetchContent)
FetchContent_Declare(CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        v2.4.1)
FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(json)

# ---------------------------------------------------------------
# cire_core: everything except the two binaries and LLVM frontend
# ---------------------------------------------------------------
add_library(cire_core STATIC
    src/graph/computation_graph.cpp
    src/error_expr/error_expr.cpp
    src/interval/interval.cpp
    src/autodiff/autodiff.cpp
    src/autodiff/rules.cpp
    src/optimizer/ibex_optimizer.cpp
    src/report/reporter.cpp
    src/driver/driver.cpp
    src/frontend/satire/lexer.cpp
    src/frontend/satire/parser.cpp
    src/frontend/satire/satire_frontend.cpp)

target_include_directories(cire_core PUBLIC src/)
target_link_libraries(cire_core PUBLIC
    CLI11::CLI11
    nlohmann_json::nlohmann_json
    ibex
    gmp
    z
    gaol
    soplex
    ultim
    gdtoa)

# ---------------------------------------------------------------
# CIRE: SATIRE binary
# ---------------------------------------------------------------
add_executable(CIRE src/bin/CIRE.cpp)
target_link_libraries(CIRE PRIVATE cire_core)

# ---------------------------------------------------------------
# CIRE_LLVM: LLVM IR binary (optional)
# ---------------------------------------------------------------
if(CIRE_ENABLE_LLVM_FRONTEND)
    find_package(LLVM REQUIRED CONFIG)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}: ${LLVM_DIR}")

    llvm_map_components_to_libnames(LLVM_LIBS
        core support irreader bitreader analysis)

    add_library(cire_llvm_frontend STATIC
        src/frontend/llvm/llvm_frontend.cpp)

    target_include_directories(cire_llvm_frontend
        PUBLIC  src/
        PRIVATE ${LLVM_INCLUDE_DIRS})

    target_compile_definitions(cire_llvm_frontend
        PRIVATE ${LLVM_DEFINITIONS}
                CIRE_LLVM_FRONTEND_ENABLED)

    target_link_libraries(cire_llvm_frontend
        PUBLIC  cire_core
        PRIVATE ${LLVM_LIBS})

    add_executable(CIRE_LLVM src/bin/CIRE_LLVM.cpp)
    target_compile_definitions(CIRE_LLVM PRIVATE CIRE_LLVM_FRONTEND_ENABLED)
    target_link_libraries(CIRE_LLVM PRIVATE cire_llvm_frontend)
endif()

# ---------------------------------------------------------------
# Tests - TODO
# ---------------------------------------------------------------
# enable_testing()
# add_subdirectory(tests)
