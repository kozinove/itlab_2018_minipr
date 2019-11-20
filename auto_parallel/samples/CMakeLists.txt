file(GLOB samples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

foreach(samp ${samples})
	get_filename_component(exe ${samp} NAME_WE)
	
	add_executable(${exe} ${samp})
	target_link_libraries(${exe} LIBS)

	if(MPI_FOUND)
		target_link_libraries(${exe} ${MPI_LIBRARIES})
	else(MPI_FOUND)
		target_link_libraries(${exe} mpi_lib)
	endif(MPI_FOUND)
	
endforeach()
