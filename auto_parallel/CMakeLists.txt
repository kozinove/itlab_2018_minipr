cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME auto_parallel)

set(MPI_INCLUDE_PATH "Enter your path here")
set(MPI_LIB_PATH "Enter your path here")


project(${PROJECT_NAME})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgomp -pthread")

include_directories(headers ${MPI_INCLUDE_PATH})

add_library(MPI_LIB STATIC IMPORTED)
set_property(TARGET MPI_LIB PROPERTY IMPORTED_LOCATION ${MPI_LIB_PATH})

add_subdirectory(sources)
add_subdirectory(samples)
