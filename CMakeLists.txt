cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(Optifuser LANGUAGES CXX CUDA)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "-o0 -g3 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-o3 -g0 -Wall")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

file(GLOB_RECURSE RENDER_SRC "include/*.h" "src/*.cpp")
file(GLOB_RECURSE APP_SRC "app/*.h" "app/*.cpp")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})

include_directories("include")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/OptiX)
  message(STATUS "Compiling with OptiX")
  include_directories("3rd_party/OptiX/include")
  link_directories("3rd_party/OptiX/lib64")
  set(OPTIX_LIBRARY optix)
  find_package(CUDA REQUIRED)
  set(
    CUDA_NVCC_FLAGS
    ${CUDA_NVCC_FLAGS};
    -O3 -std=c++11
    )
else()
  message(WARNING "Compiling without OptiX support")
  list(FILTER RENDER_SRC EXCLUDE REGEX "optix_renderer")
endif()
  
add_library(optifuser SHARED ${RENDER_SRC})
target_link_libraries(optifuser ${CUDA_LIBRARIES} ${OPENGL_LIBRARY} glfw ${GLEW_LIBRARY}
  ${ASSIMP_LIBRARIES} pthread ${OPTIXLIBRARY})

add_executable(run ${APP_SRC})
target_link_libraries(run optifuser ${CUDA_LIBRARIES} ${OPENGL_LIBRARY} glfw ${GLFW_LIBRARY}
  ${ASSIMP_LIBRARIES} pthread ${OPTIX_LIBRARY})

set_target_properties(optifuser run
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
