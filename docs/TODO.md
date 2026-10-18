# TODO

## Features

- A CMake function in the package configuration that adds the custom command of an icon, so that a project names the drawing, the sources of particular sizes and the output, and the dependencies follow from them.
- A cross build. The program runs on the build machine, so a cross build requires a build of it for the build machine and a Qt for the build machine, which the package does not yet describe.

## Tests

- The rendering of an SVG with text, which needs the fonts of the platform plugin, is not tested.
- The `.ico` is checked against the reader of Qt only. Windows itself, the resource compiler and File Explorer, is not checked automatically.
- The `.icns` is checked with `iconutil` on macOS only. The tests on Windows and Linux read the structure that the program writes.
