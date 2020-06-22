## Tools:
1. selection tools
3. tool icon
4. move tool

## canvas:
- use color inverse for brush square

## file:
- save and load functions
- export png
- read json when creating new file

## Layer and image:
- layer visability
- drag and drop layer rearrange
- rename layer in the list box
- differnet blend mode!
- layer should only handle the modifier render, the blend mode between layers should be done by Skin class

## Modifier
- better modifier viewer
- The layer overlay modifier
- separate modifier ui and modifier objects, use register/interface to create ui
- enable/disable modifier

## Others
There are a lot of coupling currently in the code. I will do decoupling after the first release come out.

split the painting functions out of the color class
