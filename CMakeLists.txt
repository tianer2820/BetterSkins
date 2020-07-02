cmake_minimum_required(VERSION 3.17)

project(MCSkinMaker)

option(DEBUG_BUILD "use debug build" ON)

add_compile_definitions(WXUSINGDLL __WXMSW__ _UNICODE)

include_directories(./wxWidgets/include/msvc ./wxWidgets/include ./json ./cpp-base64)

link_directories(wxWidgets/lib)
link_directories(wxWidgets/lib/vc_x64_dll)

if(NOT DEBUG_BUILD)
    add_compile_definitions(NDEBUG)
endif()

add_subdirectory(./cpp-base64)

add_subdirectory(./src)
