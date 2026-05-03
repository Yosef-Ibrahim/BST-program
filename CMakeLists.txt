cmake_minimum_required(VERSION 3.15)
project(BST)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "src/*.h")

add_subdirectory(external/raylib)
add_subdirectory(external/raygui)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES})

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    raylib
    raygui
    opengl32
    gdi32
    winmm
    user32
    shell32
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/external/raylib/lib/raylib.dll"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)