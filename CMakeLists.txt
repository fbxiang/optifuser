cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(Optifuser LANGUAGES CXX CUDA)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "-o0 -g3 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-o3 -g0 -Wall")

find_package(CUDA REQUIRED)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})

set(
  CUDA_NVCC_FLAGS
  ${CUDA_NVCC_FLAGS};
  -O3 -std=c++11
  )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL)
find_package(GLEW)
find_package(glfw3)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

file(GLOB_RECURSE RENDER_SRC "include/*.h" "src/*.cpp")
file(GLOB_RECURSE APP_SRC "app/*.h" "app/*.cpp")

include_directories("include")
include_directories("/home/fx/NVIDIA-OptiX-SDK-6.0.0-linux64/include")
link_directories("/home/fx/NVIDIA-OptiX-SDK-6.0.0-linux64/lib64")

add_library(optifuser SHARED ${RENDER_SRC})
target_link_libraries(optifuser ${CUDA_LIBRARIES} ${OPENGL_LIBRARY} glfw ${GLEW_LIBRARY} assimp pthread optix)

add_executable(run ${APP_SRC})
target_link_libraries(run optifuser ${CUDA_LIBRARIES} ${OPENGL_LIBRARY} glfw ${GLFW_LIBRARY} assimp pthread optix)

set_target_properties(optifuser
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
