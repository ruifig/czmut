set(SOURCE_FILES
	"library.json"
	"LICENSE"

	"src/crazygaze/mut/helpers/initializer_list"
	"src/crazygaze/mut/helpers/ministd.h"
	"src/crazygaze/mut/helpers/vaargs_to_string_array.h"

	"src/crazygaze/mut/mut.cpp"
	"src/crazygaze/mut/mut.h"
)

add_library(czmut STATIC ${SOURCE_FILES})
target_include_directories(czmut PUBLIC "./src")

if(MSVC)
	# This is needs so the code can use __cplusplus to detect the C++ version
	target_compile_options(czmut PUBLIC "/Zc:__cplusplus")
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_subdirectory(./examples)
add_subdirectory(./docs)

