# Samples
set(TARGET add add2 bf label cpuinfo)
foreach(src IN ITEMS ${TARGET})
	add_executable(${src} ${src}.cpp)
	target_link_libraries(${src} PRIVATE xbyak_aarch64)
	set_target_properties(${src} PROPERTIES
		CXX_STANDARD 11
		CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO)
endforeach()
