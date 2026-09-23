cmake_minimum_required(VERSION 3.10)
project(MyVoxelEngine)

set(CMAKE_CXX_STANDARD 17)

# GLFW
set(GLFW_ROOT "${CMAKE_SOURCE_DIR}/lib/GLFW")
include_directories(${GLFW_ROOT}/include)
link_directories(${GLFW_ROOT}/lib-vc2022)

# GLAD
include_directories(lib/GLAD/include)
file(GLOB GLAD_SRC lib/GLAD/src/*.c)

# GLM
include_directories(lib/GLM)

# FreeType-GL
set(FREETYPE_GL_ROOT "${CMAKE_SOURCE_DIR}/lib/freetype-gl")
include_directories(${FREETYPE_GL_ROOT}/build)
include_directories(${FREETYPE_GL_ROOT})
link_directories(${FREETYPE_GL_ROOT}/build/Release)

# FreeType (optional but usually required)
set(FREETYPE_ROOT "${CMAKE_SOURCE_DIR}/lib/freetype/freetype-2.13.0")
include_directories(${FREETYPE_ROOT}/include/freetype)
include_directories(${FREETYPE_ROOT}/include)
link_directories(${FREETYPE_ROOT}/objs/x64/Release)

include_directories(${CMAKE_SOURCE_DIR}/lib/glew-2.2.0/include)
link_directories(${CMAKE_SOURCE_DIR}/lib/glew-2.2.0/lib/Release/x64)


add_executable(MyVoxelEngine
    src/main.cpp
    src/graphics/Shader.cpp
    src/graphics/DebugBlockRay.cpp
    src/graphics/DebugRenderer.cpp
    src/graphics/CrosshairRenderer.cpp
    src/graphics/Camera.cpp
    src/graphics/objects/Cube.cpp
    src/graphics/objects/Cube.cpp
    src/graphics/stb_image.cpp
    src/world/World.cpp
    src/world/ChunkGenerator.cpp
    src/world/Chunk.cpp
    src/world/DroppedItem.cpp
    src/player/PlayerStats.cpp
    src/player/Player.cpp
    src/player/Item.cpp
    src/player/Crafting.cpp
    src/basics/TextRenderer.cpp
    src/ui/Basics.cpp
    src/ui/Hotbar.cpp
    src/ui/Inventory.cpp
    src/interactable/CraftingTable.cpp
    ${GLAD_SRC}
)



target_link_libraries(MyVoxelEngine freetype-gl freetype glew32 glfw3 opengl32)
