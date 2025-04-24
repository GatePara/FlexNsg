set(CMAKE_CXX_STANDARD 11)

add_executable(test_nndescent test_nndescent.cpp)
target_link_libraries(test_nndescent ${PROJECT_NAME})

add_executable(test_nsg_index test_nsg_index.cpp)
target_link_libraries(test_nsg_index ${PROJECT_NAME})

add_executable(test_nsg_index_alpha test_nsg_index_alpha.cpp)
target_link_libraries(test_nsg_index_alpha ${PROJECT_NAME})

add_executable(test_nsg_search test_nsg_search.cpp)
target_link_libraries(test_nsg_search ${PROJECT_NAME})

add_executable(test_nsg_search_analysis test_nsg_search_analysis.cpp)
target_link_libraries(test_nsg_search_analysis ${PROJECT_NAME})

add_executable(test_nsg_optimized_search test_nsg_optimized_search.cpp)
target_link_libraries(test_nsg_optimized_search ${PROJECT_NAME})

add_executable(test_platform test_platform.cpp)
