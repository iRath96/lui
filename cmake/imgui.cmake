find_package(glfw3 3.2 REQUIRED)
find_package(OpenGL REQUIRED)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/ext/imgui)
set(IMGUI_INCLUDE_DIR ${IMGUI_DIR})
file(GLOB IMGUI_SOURCES
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
file(GLOB IMGUI_HEADERS ${IMGUI_DIR}/*.h)

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_SOURCES})
target_compile_features(imgui PUBLIC cxx_std_14)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GL3W)

target_include_directories(imgui
    PUBLIC ${IMGUI_INCLUDE_DIR}
    PUBLIC ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(imgui glfw ${OPENGL_LIBRARIES})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(imgui
        "-framework CoreFoundation"
    )
else()
    target_link_libraries(imgui -ldl -lpthread)
endif()

set_target_properties(imgui PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(imgui PROPERTIES FOLDER ext)
