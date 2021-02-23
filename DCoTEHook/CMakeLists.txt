# CMakeList.txt : CMake project for SucHook, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (DCoTEHook "DCoTEHook.cpp" "DCoTEHook.h")

install (TARGETS DCoTEHook)
