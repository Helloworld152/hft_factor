add_library(mid_price_factor_plugin SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/plugins/mid_price_factor_plugin.cpp)
target_link_libraries(mid_price_factor_plugin PRIVATE hft_factor_lib)
set_target_properties(mid_price_factor_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${HFT_FACTOR_PLUGIN_OUTPUT_DIR}
    PREFIX "lib"
)

add_library(spread_factor_plugin SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/plugins/spread_factor_plugin.cpp)
target_link_libraries(spread_factor_plugin PRIVATE hft_factor_lib)
set_target_properties(spread_factor_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${HFT_FACTOR_PLUGIN_OUTPUT_DIR}
    PREFIX "lib"
)

add_library(book_imbalance_factor_plugin SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/plugins/book_imbalance_factor_plugin.cpp)
target_link_libraries(book_imbalance_factor_plugin PRIVATE hft_factor_lib)
set_target_properties(book_imbalance_factor_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${HFT_FACTOR_PLUGIN_OUTPUT_DIR}
    PREFIX "lib"
)

add_library(ret_1_tick_factor_plugin SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/plugins/ret_1_tick_factor_plugin.cpp)
target_link_libraries(ret_1_tick_factor_plugin PRIVATE hft_factor_lib)
set_target_properties(ret_1_tick_factor_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${HFT_FACTOR_PLUGIN_OUTPUT_DIR}
    PREFIX "lib"
)
