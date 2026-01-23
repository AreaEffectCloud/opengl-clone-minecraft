# Cloning Minecraft with OpenGL
I've made the voxel game like Minecraft on C++.

## Library
- cgml
- glad (glad.h)
- GLFW (glfw3.h, glfw3native.h)
  - glfw3.dll
  - libglfw3dll.a
- glm
- KHR (khrplatform.h)
- stb (stb_image.h)

## Configuration
- /assets
  - /shader: vertex, fragment shader (glsl)
  - /textures: *.png
- /block
  - block.hpp
- /gfx
  - cube_renderer
  - shader_utils
  - vertex.hpp
- /util
  - camera
  - direction
  - glad.c
  - thread_pool.hpp
- /world
  - chunk
  - world_renderer
  - world
- main_worldgen.cpp

## Compile
Move to the `bin` directory and run it.
```bash
mingw32-make.exe ; .\game.exe
```
