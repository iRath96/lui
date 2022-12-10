cmake_minimum_required(VERSION 3.11)

project(lui
  DESCRIPTION
    "Lens user interface"
  LANGUAGES
    CXX C
)

include("cmake/imgui.cmake")
include("cmake/implot.cmake")
add_subdirectory("ext/lore")

file(GLOB lui_SOURCES CONFIGURE_DEPENDS "include/lui/**/*.h" "src/*.cpp" "src/**/*.cpp")

add_executable(lui ${lui_SOURCES})
target_link_libraries(lui PRIVATE lore imgui implot)
target_include_directories(lui PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_compile_features(lui PUBLIC cxx_std_17)
