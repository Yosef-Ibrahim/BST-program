project(raygui_module)

add_library(raygui STATIC src/raygui.cpp)

target_include_directories(raygui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(raygui PRIVATE raylib)