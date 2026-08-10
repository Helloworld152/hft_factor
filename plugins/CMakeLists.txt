add_library(mid_price_factor SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/mid_price_factor_plugin.cpp)
target_link_libraries(mid_price_factor PRIVATE hft_factor_lib)
set_target_properties(mid_price_factor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${HFT_FACTOR_PLUGIN_OUTPUT_DIR}
    PREFIX "lib"
)

add_library(spread_factor SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/spread_factor_plugin.cpp)
target_link_libraries(spread_factor PRIVATE hft_factor_lib)
set_target_properties(spread_factor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${HFT_FACTOR_PLUGIN_OUTPUT_DIR}
    PREFIX "lib"
)

add_library(book_imbalance_factor SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/book_imbalance_factor_plugin.cpp)
target_link_libraries(book_imbalance_factor PRIVATE hft_factor_lib)
set_target_properties(book_imbalance_factor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${HFT_FACTOR_PLUGIN_OUTPUT_DIR}
    PREFIX "lib"
)

add_library(ret_1_tick_factor SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/ret_1_tick_factor_plugin.cpp)
target_link_libraries(ret_1_tick_factor PRIVATE hft_factor_lib)
set_target_properties(ret_1_tick_factor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${HFT_FACTOR_PLUGIN_OUTPUT_DIR}
    PREFIX "lib"
)
