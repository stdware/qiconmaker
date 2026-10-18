# qiconmaker

qiconmaker writes a Windows icon (`.ico`) or a macOS icon (`.icns`) from a drawing in SVG or PNG. A build runs it, so that a repository keeps the drawing of its icon rather than the icon files.

The images are rendered with QtSvg, the renderer with which a Qt program draws an SVG icon. The program requires Qt 6 with Qt Gui and Qt Svg, and nothing else.

## Usage

```
qiconmaker [--sizes <list>] [--png-dir <dir>] <output> <drawing> [<size>=<file>...]
```

- `output` is the file to write. Its suffix selects the format, `.ico` or `.icns`. The directory of the output is created if absent.
- `drawing` is the master drawing, an SVG (`.svg`, `.svgz`) or a PNG. It must be square.
- `<size>=<file>` gives the image of one size its own source, such as a drawing aligned to the pixel grid of 16 pixels. An SVG is rendered at that size, and a PNG must have exactly that size.
- Every other size is made from the master drawing. An SVG master is rendered at the size. A PNG master is reduced to the size, averaging the pixels, and is never enlarged, so a PNG master smaller than a size is an error.
- `--sizes` replaces the sizes of the format with a list separated by commas.
- `--png-dir` also writes each image as `<size>.png` into a directory, for inspection.

The program exits with 0 on success and 1 on an error, which it reports on the standard error. The output is written whole or not at all.

### Sizes

| Format | Sizes by default | Storage |
|---|---|---|
| `.ico` | 16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 72, 80, 96, 256 | 256 as PNG, the others as 32-bit BGRA bitmaps with an AND mask |
| `.icns` | 16, 32, 64, 128, 256, 512, 1024 | PNG, each image under every element type of its size |

The sizes of `.ico` are the union of the sizes that the title bar, the taskbar and the Start menu of Windows use at each scale. An `.icns` holds only the sizes of the table. The image of 32 pixels serves 32 points at 1x and 16 points at 2x, as `iconutil` stores an iconset.

### In CMake

```cmake
find_package(qiconmaker REQUIRED)

set(_icon "${CMAKE_CURRENT_BINARY_DIR}/app.ico")
add_custom_command(OUTPUT "${_icon}"
    COMMAND qiconmaker::qiconmaker "${_icon}" "${CMAKE_CURRENT_SOURCE_DIR}/icons/app.svg"
        "16=${CMAKE_CURRENT_SOURCE_DIR}/icons/app-16.svg"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/icons/app.svg"
        "${CMAKE_CURRENT_SOURCE_DIR}/icons/app-16.svg"
    VERBATIM
)
```

A target of the same directory that lists `${_icon}` as a source builds the icon first. `qm_add_win_rc` and `qm_add_mac_bundle` of qmsetup accept such an icon, called in the directory of the custom command.

The program creates a `QGuiApplication` for the fonts of a drawing with text, and uses the `offscreen` platform if `QT_QPA_PLATFORM` is not set, so that it runs on a build machine without a display. On Windows the DLLs of Qt must be found, for example through `PATH`.

## Building

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Qt/<version>/<compiler>
cmake --build build
cmake --install build --prefix /path/to/prefix
```

`-DQICONMAKER_BUILD_TESTS=ON` builds the tests, which `ctest --test-dir build` runs. On macOS a test checks the `.icns` with `iconutil`, because Apple publishes no specification of the format.

## License

MIT, see [LICENSE](LICENSE).
