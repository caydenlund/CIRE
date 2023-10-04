cmake_minimum_required(VERSION 3.22)
project(CIRE)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)

# Building lib_tester
add_executable(lib_tester src/lib_tester.cpp)
target_link_libraries(lib_tester PUBLIC
        Graph
        ibex)
target_include_directories(lib_tester PUBLIC
        include)


# Building Scanner and Parser
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(MyParser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h)
FLEX_TARGET(MyScanner src/lexer.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

add_executable(Foo
        ${BISON_MyParser_OUTPUTS}
        ${FLEX_MyScanner_OUTPUTS})
target_link_libraries(Foo Graph ibex)


# Building CIRE
add_executable(CIRE src/CIRE.cpp
        ${BISON_MyParser_OUTPUTS}
        ${FLEX_MyScanner_OUTPUTS})
target_link_libraries(CIRE PUBLIC Graph ibex)
target_include_directories(CIRE PUBLIC
        include)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    /usr/local/include/ibex
                    /usr/local/include/ibex/3rd
                    /usr/local/include/ibex/3rd/gaol)