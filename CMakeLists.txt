cmake_minimum_required(VERSION 3.16)

project(czmut
		VERSION 0.1
		DESCRIPTION "Catch2 inspired unit test framework for microcontrollers"
		LANGUAGES CXX)

set_property( GLOBAL PROPERTY USE_FOLDERS ON )

# Remove the MinSizeRel and RelWithDebInfo configs
# https://cmake.org/Wiki/CMake_FAQ#How_can_I_specify_my_own_configurations_.28for_generators_that_allow_it.29_.3F
if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES Debug Release)
	set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
		"Reset the configurations to what we need"
 		FORCE)
endif()

# Tweak some more things
if(MSVC)
	add_compile_options(/MP)
	# Add debug information to Release (By default, the "Release" configuration cmake creates has no debug info)
	add_compile_options("$<$<CONFIG:Release>:/Zi>")

	# https://docs.microsoft.com/en-us/cpp/build/reference/debug-generate-debug-info?view=vs-2019
	# FULL is the best option for shipping
	# FASTLINK is the best option to use during development
	add_link_options("$<$<CONFIG:Debug>:/DEBUG:FASTLINK>")
	add_link_options("$<$<CONFIG:Release>:/DEBUG:FULL>")
endif()

set(CMAKE_CXX_STANDARD 17)

include(cmake/utils.cmake)

# Set all targets to use Unicode
add_definitions(-DUNICODE -D_UNICODE)
add_subdirectory(./lib)

