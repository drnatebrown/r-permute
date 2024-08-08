set(DS_SOURCES FL_table.hpp static_column.hpp index_pq.hpp LF_table.hpp)

add_library(ds OBJECT ${DS_SOURCES})
target_link_libraries(ds common sdsl)