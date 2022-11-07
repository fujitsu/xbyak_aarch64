cmake_minimum_required (VERSION 3.8)

project(xbyak_aarch64
	VERSION 1.0.0
	LANGUAGES CXX)

option(
	SAMPLE
	"Build xbyak_aarch64 samples"
	OFF
)
option(
	TEST
	"Build xbyak_aarch64 tests"
	OFF
)

if(NOT DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(xbyak_aarch64 STATIC src/xbyak_aarch64_impl.cpp src/util_impl.cpp)
target_include_directories(xbyak_aarch64 PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xbyak_aarch64/>
	$<INSTALL_INTERFACE:$CMAKE_INSTALL_DIR/include>)
set_target_properties(xbyak_aarch64 PROPERTIES
	POSITION_INDEPENDENT_CODE ON)

if(MSVC)
	if(MCL_MSVC_RUNTIME_DLL)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /MD /Oy /Ox /EHsc /GS- /Zi /DNDEBUG")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /MDd")
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /MT /Oy /Ox /EHsc /GS- /Zi /DNDEBUG")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /MTd")
	endif()

	target_compile_definitions(xbyak_aarch64 PUBLIC NOMINMAX)
	set(MCL_COMPILE_OPTIONS /W4)
	target_compile_options(xbyak_aarch64 PRIVATE ${MCL_COMPILE_OPTIONS})
else()
	# Set compiler flags for warnings
	set(MCL_COMPILE_OPTIONS -Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align
		-Wwrite-strings -Wfloat-equal -Wpointer-arith -DNDEBUG -O3 -fPIC)

	target_compile_options(xbyak_aarch64 PRIVATE ${MCL_COMPILE_OPTIONS})
	set_target_properties(xbyak_aarch64 PROPERTIES
		CXX_STANDARD 11
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO)
	target_compile_features(xbyak_aarch64 PUBLIC cxx_std_11)
endif()

install(TARGETS xbyak_aarch64
	EXPORT xbyak_aarch64Targets
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

install(DIRECTORY include/xbyak_aarch64
	DESTINATION include
	FILES_MATCHING PATTERN "*.h"
)

install(EXPORT xbyak_aarch64Targets
	FILE xbyak_aarch64Targets.cmake
	NAMESPACE xbyak_aarch64::
	DESTINATION lib/cmake/xbyak_aarch64)

# support local build-tree export to allow import from external projects
export(EXPORT xbyak_aarch64Targets
	FILE xbyak_aarch64Targets.cmake
	NAMESPACE xbyak_aarch64::)
set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(PACKAGE xbyak_aarch64 )


# Tests
if(TEST)
	enable_testing()
	add_subdirectory(test)
endif()

if(SAMPLE)
	# sample code
	add_subdirectory(sample)
endif()
