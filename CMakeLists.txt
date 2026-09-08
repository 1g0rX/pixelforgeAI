cmake_minimum_required(VERSION 3.20)
project(PixelForgeAI VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -pedantic -Wno-unused-parameter -Wno-missing-field-initializers)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Core Library Sources
set(PIXELFORGE_SOURCES
    third_party/gifenc/gifenc.c
    src/sprite/Palette.cpp
    src/sprite/Layer.cpp
    src/sprite/Region.cpp
    src/sprite/Sprite.cpp
    src/sprite/SpriteState.cpp
    src/sprite/SpriteAnalyzer.cpp
    src/sprite/History.cpp
    src/tools/ToolRegistry.cpp
    src/tools/PrimitiveTools.cpp
    src/tools/SemanticTools.cpp
    src/animation/Animation.cpp
    src/animation/FrameDiffer.cpp
    src/animation/SpriteSheet.cpp
    src/animation/AnimationTools.cpp
    src/io/PngIO.cpp
    src/io/GifIO.cpp
    src/io/SpriteSheetIO.cpp
    src/agent/MockAgentBackend.cpp
    src/agent/HttpAgentBackend.cpp
    src/agent/AgentEvaluator.cpp
    src/agent/AgentLoop.cpp
    src/cli/CliApp.cpp
)

add_library(pixelforge_core STATIC ${PIXELFORGE_SOURCES})

# Executable
add_executable(pixel-forge src/main.cpp)
target_link_libraries(pixel-forge PRIVATE pixelforge_core)

# Unit Tests
set(TEST_SOURCES
    tests/test_main.cpp
    tests/test_sprite.cpp
    tests/test_palette.cpp
    tests/test_regions.cpp
    tests/test_semantic_tools.cpp
    tests/test_history.cpp
    tests/test_animation.cpp
    tests/test_spritesheet.cpp
    tests/test_tools_registry.cpp
    tests/test_text_repr.cpp
    tests/test_agent_loop.cpp
)

add_executable(pixel-forge-tests ${TEST_SOURCES})
target_include_directories(pixel-forge-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(pixel-forge-tests PRIVATE pixelforge_core)

enable_testing()
add_test(NAME AllTests COMMAND pixel-forge-tests)

# Example
add_executable(pixel-forge-example examples/basic_api_usage.cpp)
target_link_libraries(pixel-forge-example PRIVATE pixelforge_core)
