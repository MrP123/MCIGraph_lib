# Emscripten
Webassembly builds are possible by following the raylib tutorial on the topic (https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5))
Separate compilation of raylib for the web is only required once.

## shell-file
The wasm build of the game is embedded into a (minimal) HTML-shell providing the required `canvas` element as a drawing target. An example is given in the `web` directory of the repo.

## Building with emscripten
Make sure emscripten is activated using
```sh
cd C:\emsdk
./emsdk activate latest
```

To ensure that most examples created with `MCIGraph` for the desktop platform work the stack size of the wasm build needs to be set at approximately 2MB. This can be achieved with:
```sh
emcc -o my_project.html ./my_project.cpp -Wall -std=c++17 -sDEFAULT_TO_CXX -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os -I. -I ./thirdparty/raylib-5.0/src -I ./thirdparty/raylib-5.0/src/external -L. -L ./thirdparty/raylib-5.0/src/exernal -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=64MB -s STACK_SIZE=2MB -s FORCE_FILESYSTEM=1 --preload-file tiles --shell-file ./web/my_shell.html ./web/libraylib.a -DPLATFORM_WEB -s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]'-s EXPORTED_RUNTIME_METHODS=ccall
```
The if multiple `*.cpp` files are used in the project those have to be specified in the usual fashion when using gcc/g++. As stated in the given tutorial make sure that raylib is at the specified path and that raylib HTML5 version is provided. The resource folder is specified with the `--preload-file` option, though one needs to be aware that all resources in this folder are added to the build, no matter if they are actually used in the game.

If debug symbols are desired one needs to provide the `-g` and `-gsource-map` options to `emcc`, usually after the optimization flag.

## Running the emscripten build
For safety reasons no browser allows running local files, therefore a local webserver is required to run the wasm build. The simplest way to do this is to use the `emrun` tool provided by emscripten:
```sh
emrun my_project.html
```
Alternatively one can use any other webserver, e.g. call `python -m http.server` in the directory of the wasm build and then open `http://localhost:8000/my_project.html` in a browser.