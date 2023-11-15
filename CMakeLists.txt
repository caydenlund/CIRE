cmake_minimum_required(VERSION 3.22)
project(CIRE)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)

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

# Building lib_tester
add_executable(lib_tester src/lib_tester.cpp)
target_link_libraries(lib_tester PUBLIC
        Graph
        ibex)

# Building CIRE
add_executable(CIRE src/CIRE.cpp)
target_link_libraries(CIRE PUBLIC
        Graph
        ibex)

