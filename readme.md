# Optifuser Renderer
My personal project on rendering with OpenGL and OptiX. It is at very early
stage, so please do not consider using it since I plan to change everything.

# Build guide
I created a very simple CMakeLists file that anyone should be able to
understand. You will need to modify it to suit your own needs.

# Supported features
GLSL vertex/fragment shaders are supported. OptiX shaders are coded but not
integrated into the main program yet. See `app/main.cpp` for a use case.
