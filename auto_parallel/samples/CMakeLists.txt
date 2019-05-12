file(GLOB samples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

foreach(samp ${samples})
	get_filename_component(exe ${samp} NAME_WE)
	
	add_executable(${exe} ${samp})
	target_link_libraries(${exe} LIBS MPI_LIB)
endforeach()
