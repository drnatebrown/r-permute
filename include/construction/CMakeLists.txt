set(CONST_SOURCES construction.hpp deterministic.hpp randomized.hpp)

add_library(construction OBJECT ${CONST_SOURCES})
target_link_libraries(construction ds common sdsl)