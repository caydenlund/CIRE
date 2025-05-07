# Building Scanner and Parser
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

set(IBEX_INSTALL_DIR "/usr/local" CACHE PATH "Ibex installation directory")

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..
    ${IBEX_INSTALL_DIR}/include
    ${IBEX_INSTALL_DIR}/include/ibex
    ${IBEX_INSTALL_DIR}/include/ibex/3rd)

link_directories(${IBEX_INSTALL_DIR}/lib
    ${IBEX_INSTALL_DIR}/lib/ibex/3rd)

BISON_TARGET(MyParser
        parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h)
FLEX_TARGET(MyScanner
        lexer.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

add_library(Graph SHARED
        Logging.cpp
        Node.cpp
        SymbolTable.cpp
        ErrorAnalyzer.cpp
        IBEXInterface.cpp
        Graph.cpp
        ${BISON_MyParser_OUTPUTS}
        ${FLEX_MyScanner_OUTPUTS})
