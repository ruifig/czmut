
# Remember x86/x64
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET( EX_PLATFORM 64)
    SET( EX_PLATFORM_NAME "x64")
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET( EX_PLATFORM 32)
    SET( EX_PLATFORM_NAME "x86")
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

function(cz_setCommonBinaryProperties target_name sub_folder)
	set_target_properties( ${target_name}
		PROPERTIES

		#ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
		#LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin${sub_folder}$<$<CONFIG:DummyConfigName>:>
		VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin${sub_folder}$<$<CONFIG:DummyConfigName>:>

		# Output file name
		#OUTPUT_NAME ${target_name}_${EX_PLATFORM_NAME}_$<CONFIG>
		OUTPUT_NAME ${target_name}_$<CONFIG>
	)
endfunction()


# Sets the IDE folder for all the files in the specified target so they match the directory tree structure
# Based on what I found at https://stackoverflow.com/questions/33808087/cmake-how-to-create-visual-studio-filters
function(cz_setIdeFolders target_name)

	get_target_property( _srcs ${target_name} SOURCES)

	foreach(_source IN LISTS _srcs)
		get_filename_component(_source_path "${_source}" PATH)
		string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_group_path}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()

endfunction()

