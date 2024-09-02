# Building Scanner and Parser
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..
            /usr/local/include
            /usr/local/include/ibex
            /usr/local/include/soplex
            /usr/local/include/ibex/3rd
            /usr/local/include/ibex/3rd/gaol)
link_directories(/usr/local/lib
                /usr/local/lib64)


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