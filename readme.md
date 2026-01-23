# Cloning Minecraft with OpenGL
I've made the voxel game like Minecraft on C++.

## Library
- cgml
- glad (glad.h)
- GLFW (glfw3.h, glfw3native.h)
- glm
- KHR (khrplatform.h)
- stb (stb_image.h)

## Configuration
- /assets
  - shader: vertex, fragment shader (glsl)
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
