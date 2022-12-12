set(IMPLOT_DIR ${CMAKE_SOURCE_DIR}/ext/implot)
set(IMPLOT_INCLUDE_DIR ${IMPLOT_DIR})
file(GLOB IMPLOT_SOURCES ${IMPLOT_DIR}/*.cpp)
file(GLOB IMPLOT_HEADERS ${IMPLOT_DIR}/*.h)

add_library(implot STATIC ${IMPLOT_SOURCES} ${IMPLOT_SOURCES})
target_compile_features(implot PUBLIC cxx_std_14)

add_definitions(-DIMPLOT_IMPL_OPENGL_LOADER_GL3W)

target_include_directories(implot
    PUBLIC ${IMPLOT_INCLUDE_DIR}
    PUBLIC ${OPENGL_INCLUDE_DIR}
    PRIVATE ${IMGUI_INCLUDE_DIR}
)

set_target_properties(implot PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(implot PROPERTIES FOLDER ext)
