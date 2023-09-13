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

include_directories(/usr/local/include/ibex
                    /usr/local/include/ibex/3rd
                    /usr/local/include/ibex/3rd/gaol)