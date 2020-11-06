cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)

project(ICONEX C)
enable_testing()

find_package(AnyDSL_runtime REQUIRED)
include_directories(${AnyDSL_runtime_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
find_package(NetCDF REQUIRED)

set(BACKEND ${BACKEND} CACHE STRING "select the backend from the following: CPU, AVX, AMDGPU, NVVM, CUDA, OPENCL")
if(NOT BACKEND)
  set(BACKEND cpu CACHE STRING "select the backend from the following: CPU, AVX, AMDGPU, NVVM, CUDA, OPENCL" FORCE)
endif()
string(TOLOWER "${BACKEND}" BACKEND)
message(STATUS "Selected backend: ${BACKEND}")
set(ENV{ICONEX_BENCH} "${BACKEND}")

set(BACKEND_FILE src/be/backend_${BACKEND}.impala)

if(BACKEND STREQUAL "cpu" OR BACKEND STREQUAL "avx")
    set(DEVICE "cpu")
    add_compile_definitions(CPU_DEVICE)
else()
    set(DEVICE "acc")
    add_compile_definitions(ACC_DEVICE)
endif()

if(BACKEND STREQUAL "cpu" OR BACKEND STREQUAL "avx")
  set(NO_OF_THREADS 4 CACHE STRING "Number of threads to use. Get a good measure with `nproc`.")
  add_compile_definitions(NUM_OF_THREADS=${NO_OF_THREADS})
else()
  unset(NO_OF_THREADS CACHE)
endif()

set(CLANG_FLAGS -march=native)
set(IMPALA_FLAGS --log-level info)
set(ANYDSL_RUNTIME_LIBRARIES ${AnyDSL_runtime_LIBRARIES})

set(CLIMATE_FILES
    src/be/device_${DEVICE}.impala
    src/be/backend_${BACKEND}.impala
    src/grid.impala
    src/ops.impala
    src/utils.impala
)

anydsl_runtime_wrap(ICONEX_OBJS
    CLANG_FLAGS ${CLANG_FLAGS}
    IMPALA_FLAGS ${IMPALA_FLAGS}
    NAME iconex
    FILES ${CLIMATE_FILES})

add_executable(global_div src/utils.c src/grid.c tests/global_div.c ${ICONEX_OBJS})
target_include_directories(global_div PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(global_div ${ANYDSL_RUNTIME_LIBRARIES} ${NETCDF_LIBRARIES_C})

add_executable(local_vel_filter src/utils.c src/grid.c tests/local_vel_filter.c ${ICONEX_OBJS})
target_include_directories(local_vel_filter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(local_vel_filter ${ANYDSL_RUNTIME_LIBRARIES} ${NETCDF_LIBRARIES_C})

include(ExternalProject)

# Download file
ExternalProject_Add(
    gridfile
    PREFIX "${CMAKE_BINARY_DIR}"
    URL http://icon-downloads.mpimet.mpg.de/grids/public/mpim/0019/icon_grid_0019_R02B05_G.nc
    DOWNLOAD_NO_EXTRACT 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
add_dependencies(global_div gridfile)
add_dependencies(local_vel_filter gridfile)

set(env_icnoex_bench_sh ${CMAKE_CURRENT_BINARY_DIR}/bench_env.sh)
file(WRITE ${env_icnoex_bench_sh} "#!/usr/bin/env bash\nexport ICONEX_BENCH=\"${BACKEND}\"")
execute_process(COMMAND chmod a+x ${env_icnoex_bench_sh} RESULT_VARIABLE res)

add_custom_target(check
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/scripts/benchmark.sh ${CMAKE_CURRENT_BINARY_DIR}/benchmark.sh
  COMMAND sh ${CMAKE_CURRENT_BINARY_DIR}/benchmark.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS global_div
)

