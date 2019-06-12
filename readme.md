# Optifuser Renderer
My personal project on rendering with OpenGL and OptiX. It is at very early
stage, so please do not consider using it since I plan to change everything.

# Build guide
## Dependencies
This library depends on `glew`, `glfw3` and `assimp`, with optional dependencies
of `cuda` and `OptiX`.

For a basic install, make sure you have the libraries installed. For example, on
Ubuntu you may use
```bash
sudo apt install libglew-dev libglfw3-dev libassimp-dev
```

For Cuda installation please refer to NVIDIA's official instructions. To install
OptiX, you need to download OptiX from NVIDIA, and then either put it at
`3rd_party/OptiX` folder or create a simlink at `3rd_party/OptiX` to its actual
installation location. Make sure `3rd_party/OptiX/include` contains its header
files and `3rd_party/OptiX/lib64` contains the library.

Next compile the project by
```bash
mkdir build && cd build
cmake ..
make
```

If OptiX is placed properly, the project will be compiled with OptiX.

# Supported features
Please note that currently, working OptiX shader files exist but they are not
compiled. So only OpenGL/GLSL renderer works at the moment. See `app/main.cpp`
for a use case. OptiX will be supported based on feature requests or our needs.
