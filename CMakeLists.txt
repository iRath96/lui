cmake_minimum_required(VERSION 3.11)

project(lui
  DESCRIPTION
    "Lens user interface"
  LANGUAGES
    CXX C
)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

include("cmake/detect_emscripten.cmake")
include("cmake/imgui.cmake")
include("cmake/implot.cmake")
add_subdirectory("ext/lore")

file(GLOB lui_SOURCES CONFIGURE_DEPENDS "include/lui/**/*.h" "src/*.cpp" "src/**/*.cpp")

add_executable(lui ${lui_SOURCES})
target_link_libraries(lui PRIVATE lore imgui implot)
target_include_directories(lui PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(lui PUBLIC cxx_std_17)

file(COPY data DESTINATION ${CMAKE_BINARY_DIR})
file(COPY /System/Library/Fonts/SFNS.ttf DESTINATION ${CMAKE_BINARY_DIR}/data/fonts)
file(COPY /System/Library/Fonts/Monaco.ttf DESTINATION ${CMAKE_BINARY_DIR}/data/fonts)
file(COPY ${lore_DATA} DESTINATION ${CMAKE_BINARY_DIR})

if(CMAKE_CXX_COMPILER_ID STREQUAL "Emscripten")
  set_target_properties(
    lui
    PROPERTIES
      OUTPUT_NAME "lui"
      SUFFIX ".html"
  )
  
  target_link_options(lui PUBLIC
    --preload-file "data"
    "-s WASM=1"
    "-s ALLOW_MEMORY_GROWTH=1"
    "-s NO_EXIT_RUNTIME=0"
    "-s ASSERTIONS=1"
  )
endif()