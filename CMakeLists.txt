cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(Optifuser LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-o0 -g3 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-o3 -g0 -Wall")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

file(GLOB_RECURSE RENDER_SRC "include/*.h" "src/*.cpp")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
set(OPTIX_HOME "NOT FOUND" CACHE FILEPATH "Home to OptiX")

if (GLFW_LIBRARY)
else()
  set(GLEW_LIBRARY GLEW)
endif()

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)
include_directories("3rd_party/imgui")
include_directories("3rd_party/imgui/examples")
include_directories("3rd_party/spdlog/include")
file(GLOB GUI_SRC "3rd_party/imgui/*.cpp"
  "3rd_party/imgui/examples/imgui_impl_glfw.cpp"
  "3rd_party/imgui/examples/imgui_impl_opengl3.cpp" )

add_subdirectory("3rd_party/glfw")
add_subdirectory("3rd_party/assimp")
add_subdirectory("3rd_party/spdlog")

include_directories("include")

if(EXISTS ${OPTIX_HOME})
  message(STATUS "Compiling with OptiX")
  include_directories("${OPTIX_HOME}/include")
  link_directories("${OPTIX_HOME}/lib64")
  add_definitions(-D_USE_OPTIX)

  set(OPTIX_LIBRARY optix)
  find_package(CUDA REQUIRED)
  set(
    CUDA_NVCC_FLAGS
    ${CUDA_NVCC_FLAGS};
    -O3 -std=c++11
    )

  file(GLOB_RECURSE CUDA_FILES "optix_shader/*.cu")
  cuda_compile_ptx(cuda_ptx_files ${CUDA_FILES} TARGET_PATH "${CMAKE_BINARY_DIR}/ptx")
  add_custom_target(ptx ALL DEPENDS ${cuda_ptx_files} ${PTX_SRC} SOURCES ${PTX_SRC})

  add_custom_target(ptx_dir COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/ptx"
    COMMAND ${CMAKE_COMMAND} -E copy ${cuda_ptx_files} "${CMAKE_BINARY_DIR}/ptx")

  add_executable(test_optix app/test_optix.cpp ${GUI_SRC})
  target_link_libraries(test_optix optifuser ${CUDA_LIBRARIES} ${OPENGL_LIBRARY} glfw ${GLFW_LIBRARY}
    assimp pthread ${OPTIX_LIBRARY} stdc++fs spdlog)

  add_dependencies(ptx_dir ptx)
  add_dependencies(test_optix ptx_dir)

else()
  message(WARNING "Compiling without OptiX support")
  list(FILTER RENDER_SRC EXCLUDE REGEX "optix_renderer")
endif()
 
add_library(optifuser SHARED ${RENDER_SRC} ${GUI_SRC})
target_link_libraries(optifuser ${CUDA_LIBRARIES} ${OPENGL_LIBRARY} glfw ${GLEW_LIBRARY}
  assimp pthread ${OPTIXLIBRARY})

add_executable(test_optifuser app/main.cpp ${GUI_SRC})
target_link_libraries(test_optifuser optifuser ${CUDA_LIBRARIES} ${OPENGL_LIBRARY} glfw ${GLFW_LIBRARY}
  assimp pthread ${OPTIX_LIBRARY} stdc++fs spdlog)

set_target_properties(optifuser test_optifuser
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)
