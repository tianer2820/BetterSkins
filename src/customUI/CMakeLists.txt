
add_subdirectory(colorPickers)
add_subdirectory(layerModifierUIs)

add_library(customUI threeSplitter.cpp skinBrowser.cpp)
target_link_libraries(customUI ColorPickers layerModifierUIs)