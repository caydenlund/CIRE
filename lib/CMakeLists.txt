add_library(Graph Node.cpp Graph.cpp)

target_link_libraries(Graph PUBLIC ibex)

include_directories(/usr/local/include/ibex
        /usr/local/include/ibex/3rd
        /usr/local/include/ibex/3rd/gaol)