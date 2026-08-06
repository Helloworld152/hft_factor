add_executable(read_factor_shm read_factor_shm.cpp)
target_link_libraries(read_factor_shm PRIVATE hft_factor_lib)
set_target_properties(read_factor_shm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin
)
