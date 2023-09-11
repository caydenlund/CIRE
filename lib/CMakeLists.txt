add_library(Node Node.cpp)

target_link_libraries(Node PUBLIC ibex)

include_directories(/usr/local/include/ibex
        /usr/local/include/ibex/3rd
        /usr/local/include/ibex/3rd/gaol)