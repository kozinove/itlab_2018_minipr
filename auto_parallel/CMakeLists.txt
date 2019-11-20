cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME auto_parallel)

project(${PROJECT_NAME})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgomp -pthread")

find_package(MPI)
if(NOT MPI_FOUND)
	message(STATUS "MPI not found")
	set(MPI_INCLUDE_PATH "")
	set(MPI_LIB_PATH "")
	if((MPI_INCLUDE_PATH STREQUAL "") OR (MPI_LIB_PATH STREQUAL ""))
		message(FATAL_ERROR "You should set MPI_INCLUDE_PATH and MPI_LIB_PATH in CMakeLists.txt")
	endif((MPI_INCLUDE_PATH STREQUAL "") OR (MPI_LIB_PATH STREQUAL ""))
	add_library(mpi_lib STATIC IMPORTED)
	set_property(TARGET mpi_lib PROPERTY IMPORTED_LOCATION ${MPI_LIB_PATH})
endif(NOT MPI_FOUND)

include_directories(headers ${MPI_INCLUDE_PATH})

add_subdirectory(sources)
add_subdirectory(samples)
