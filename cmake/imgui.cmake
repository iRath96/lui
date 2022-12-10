find_package(OpenGL REQUIRED)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/ext/imgui)
set(IMGUI_INCLUDE_DIR ${IMGUI_DIR})

if(CMAKE_CXX_COMPILER_ID STREQUAL "Emscripten")
    file(GLOB IMGUI_SOURCES
        ${IMGUI_DIR}/*.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
else()
    find_package(glfw3 3.2 REQUIRED)

    file(GLOB IMGUI_SOURCES
        ${IMGUI_DIR}/*.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
endif()

file(GLOB IMGUI_HEADERS ${IMGUI_DIR}/*.h)

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_SOURCES})
target_compile_features(imgui PUBLIC cxx_std_14)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GL3W)

target_include_directories(imgui
    PUBLIC ${IMGUI_INCLUDE_DIR}
    PUBLIC ${IMGUI_INCLUDE_DIR}/backends
    PUBLIC ${OPENGL_INCLUDE_DIR}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Emscripten")
    target_compile_options(imgui PUBLIC
        -s USE_SDL=2
    )
    target_link_options(imgui PUBLIC
        -s USE_SDL=2
        --shell-file ${CMAKE_CURRENT_LIST_DIR}/imgui.html
    )
else()
    target_link_libraries(imgui glfw ${OPENGL_LIBRARIES})

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        target_link_libraries(imgui
            "-framework CoreFoundation"
        )
    else()
        target_link_libraries(imgui -ldl -lpthread)
    endif()
endif()

set_target_properties(imgui PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(imgui PROPERTIES FOLDER ext)
