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

find_library(DINPUTLIB dinput8.lib PATHS "DirectInput")
if (NOT DINPUTLIB)
	message (FATAL_ERROR "DINPUTLIB library not found")
else ()
	message (STATUS "DINPUTLIB found at ${DINPUTLIB}")
	target_link_libraries(HookDll ${DINPUTLIB})
endif ()

find_library(D3D9LIB d3d9.lib PATHS "Direct3D9")
if (NOT D3D9LIB)
	message (FATAL_ERROR "D3D9LIB library not found")
else ()
	message (STATUS "D3D9LIB found at ${D3D9LIB}")
	target_link_libraries(HookDll ${D3D9LIB})
endif ()

find_library(DXGUIDLIB dxguid.lib PATHS "DirectInput")
if (NOT DXGUIDLIB)
	message (FATAL_ERROR "DXGUIDLIB library not found")
else ()
	message (STATUS "DXGUIDLIB found at ${DXGUIDLIB}")
	target_link_libraries(HookDll ${DXGUIDLIB})
endif ()

install (TARGETS HookDll)
install (FILES ${MINHOOKDLL} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
