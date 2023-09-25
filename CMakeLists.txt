cmake_minimum_required(VERSION 3.22)
project(CIRE)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)
add_subdirectory(tests)

add_executable(cire src/cire.cpp)
target_link_libraries(cire PUBLIC
        Node
        ibex)
target_include_directories(cire PUBLIC
        include)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(MyParser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h)
FLEX_TARGET(MyScanner src/lexer.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

add_executable(Foo
        ${BISON_MyParser_OUTPUTS}
        ${FLEX_MyScanner_OUTPUTS})
target_link_libraries(Foo ${FLEX_LIBRARIES} Node ibex)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    /usr/local/include/ibex
                    /usr/local/include/ibex/3rd
                    /usr/local/include/ibex/3rd/gaol)