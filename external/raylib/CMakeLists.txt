project(raylib_module)

file(GLOB RAYLIB_HEADERS "include/*.h")

add_library(raylib SHARED IMPORTED GLOBAL)

set_target_properties(raylib PROPERTIES
    IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/libraylibdll.a" 
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/raylib.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)