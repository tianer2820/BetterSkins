# BetterSkins
Another Mincraft skin editor.

**This program is still under heavy development, any bugs can occur. Use at your own risk.**

## Overview
This this another skin maker for minecraft. The purpose is to replace and improve the _MCSkin3D_, which is no longer being developmented.

This skin maker will be offline, and will mimic the Blender editing style, i.e. layers and modifiers. This will allow the creation of templates and can create good looking skins very fast.

## Usage
No release version yet, you will need to compile the source code. When done, run the "Test.exe". I will rename it in the first realease.

## Build
Requirements:
- wxWidgets (3.1.3+)
- json by nlohmann (already included in the code) [Link](https://github.com/nlohmann/json)

After installing the requirements, you may need to tweak `./CMakeLists.txt`. All lib configurations are done in this top level cmakelist. You shouldn't need to modify anything in the `src` folder.

Then build the project with CMake. When finished, copy the "Test.exe" to appropriate folder, and copy the `resources` folder to the same folder. The executable will read icons and configs from this folder.

Notice: If you are using dll build of wxWidgets, you will also need to copy the required dlls to the folder, together with the executable file.

## Development
All source files are in the `src` folder. There are some subfolders:
- color: contains the color class, is used by color picker ctrls, and can perform basic color space conversion
- customUI: contains all UI classes.
- dataStructure: the data structures used when making skin documents. the Layer class, skin class, modifier class etc.

**There will still be huge changes, be aware**
