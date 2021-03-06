cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(Optifuser LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-o0 -g3 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-o3 -g0 -Wall")

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(ASSIMP REQUIRED)
find_package(spdlog REQUIRED)

file(GLOB_RECURSE RENDER_SRC "include/*.h" "src/*.cpp")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
set(OPTIX_HOME "NOT FOUND" CACHE FILEPATH "Home to OptiX")
set(USE_PRECOMPILED_PTX FALSE CACHE BOOL "Whether to use pre-built OptiX shader files")

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)
include_directories(
    "3rd_party/imgui"
    "3rd_party/imgui/examples"
    "3rd_party/ImGuizmo"
    )
include_directories()
file(GLOB GUI_SRC "3rd_party/imgui/*.cpp"
  "3rd_party/ImGuizmo/*.cpp"
  "3rd_party/imgui/backends/imgui_impl_opengl3.cpp"
  "3rd_party/imgui/backends/imgui_impl_glfw.cpp")

find_package(glfw3 REQUIRED)

include_directories("include")

if(EXISTS ${OPTIX_HOME})
  message(STATUS "Compiling with OptiX")
  include_directories("${OPTIX_HOME}/include")
  link_directories("${OPTIX_HOME}/lib64")
  add_definitions(-D_USE_OPTIX)
  set(OPTIX_LIBRARY optix)

  if (${USE_PRECOMPILED_PTX})
    message(STATUS "Use Precompiled shaders")
    add_custom_target(ptx_dir COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/ptx ${CMAKE_BINARY_DIR})
  else()
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

    add_dependencies(ptx_dir ptx)
  endif()

  add_executable(test_optix app/test_optix.cpp ${GUI_SRC})
  target_link_libraries(test_optix optifuser ${CUDA_LIBRARIES} ${OPENGL_LIBRARY} glfw3
    pthread ${OPTIX_LIBRARY} stdc++fs)
  add_dependencies(test_optix ptx_dir)
else()
  message(WARNING "Compiling without OptiX support")
  list(FILTER RENDER_SRC EXCLUDE REGEX "optix_renderer")
endif()

add_library(optifuser STATIC ${RENDER_SRC} ${GUI_SRC})
target_link_libraries(optifuser ${CUDA_LIBRARIES} ${OPENGL_LIBRARY} GLEW glfw3
  ${ASSIMP_LIBRARIES}
  pthread ${OPTIX_LIBRARY} ${SPDLOG_LIBRARIES})

add_executable(test_optifuser app/main.cpp ${GUI_SRC})
target_link_libraries(test_optifuser optifuser ${CUDA_LIBRARIES} ${OPENGL_LIBRARY} GLEW glfw3
  pthread ${OPTIX_LIBRARY} stdc++fs)


set_target_properties(optifuser test_optifuser
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
  )
