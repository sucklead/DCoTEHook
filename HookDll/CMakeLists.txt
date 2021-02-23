# CMakeList.txt : CMake project for SucHook, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (HookDll SHARED "HookDll.cpp" "HookDll.h")

find_library(MINHOOKLIB MinHook.x86.lib PATHS "MinHook")
if (NOT MINHOOKLIB)
	message (FATAL_ERROR "MINHOOKLIB library not found")
else ()
	message (STATUS "MINHOOKLIB found at ${MINHOOKLIB}")
	target_link_libraries(HookDll ${MINHOOKLIB})
endif ()

find_file(MINHOOKDLL MinHook.x86.dll PATHS "MinHook")
if (NOT MINHOOKDLL)
	message (FATAL_ERROR "MINHOOKDLL dll not found")
else ()
	message (STATUS "MINHOOKDLL found at ${MINHOOKDLL}")
endif ()


install (TARGETS HookDll)
install (FILES ${MINHOOKDLL} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
