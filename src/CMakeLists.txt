add_subdirectory(color)
add_subdirectory(customUI)
add_subdirectory(dataStructure)
add_library(ImageOperations imageOperations.cpp)


add_executable(Main WIN32 main.cpp)


target_link_libraries(Main customUI color ImageOperations dataStructure)
target_link_libraries(Main base64)
