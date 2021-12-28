# Pig's Castle Game

A small 2d sidescroller game written in C++ with SDL2.

![Pigs Castle](etc/example.gif)


## Map editor

A WYSIWYG map editor is provided for level design. It is fully developed over
SDL, so no extra dependencies are required.

![Map editor](etc/example_mapeditor.gif)


## How to build

Make sure you have SDL installed. For example, in Ubuntu systems:

```
apt install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev
```

Building with CMake:

```
mkdir build/
cd build/
cmake ..
cmake --build .
```

## Special thanks
- [Pixel Frog](https://twitter.com/_pixelfrog): Some Pixel art (background and characters)
- [Sérgio](https://github.com/sergiogibe/): Background music & many cool suggestions
