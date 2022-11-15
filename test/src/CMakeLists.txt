add_executable(build_constructor build_constructor.cpp)
target_link_libraries(build_constructor sdsl malloc_count divsufsort divsufsort64)
target_include_directories(build_constructor PUBLIC    "../../include/ds" 
                                        "../../include/common"
                                        "../../include/construction" 
                                        )
target_compile_options(build_constructor PUBLIC "-std=c++17")