cmake_minimum_required(VERSION 3.16)
project(LiteEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER /home/user/Programs/llvm-project-llvmorg-12.0.0/build/bin/clang++)

set(CMAKE_UNITY_BUILD 0)

add_compile_definitions(UNITY_BUILD=0)
add_compile_definitions(DEBUG=1)
add_compile_definitions(PROJECT_NAME="default project name")
add_compile_definitions(ENGINE_NAME="LiteEngine")
add_compile_definitions(ENGINE_VERSION=1)
add_compile_definitions(USE_VULKAN_VALIDATION_LAYERS=1)
add_compile_definitions(WITH_OSS=1)

include_directories(Source/UserInterface)
include_directories(Source/CoreFiles)
include_directories(Source/Audio)
include_directories(Source/Networking)
include_directories(Source/Physics)
include_directories(Source/Rendering)
include_directories(Source/UserInterface/backends)
include_directories(Source/Rendering/glfw-3.3.3/include)

add_subdirectory(Source/Rendering/glfw-3.3.3)
add_subdirectory(Source/Format)

add_executable(LiteEngine Source/CoreFiles/main.cpp Source/CoreFiles/Simd.hpp Source/CoreFiles/Simd.cpp Source/CoreFiles/String.hpp Source/CoreFiles/String.cpp Source/CoreFiles/Math.hpp Source/CoreFiles/Memory.hpp Source/CoreFiles/Definitions.hpp Source/CoreFiles/Vector.hpp Source/CoreFiles/Vector.cpp Source/CoreFiles/Array.hpp Source/CoreFiles/Array.cpp Source/CoreFiles/Interface/Tick.hpp Source/CoreFiles/Interface/Tick.cpp Source/CoreFiles/SmartPointer.hpp Source/CoreFiles/SmartPointer.cpp Source/CoreFiles/Atomic.hpp Source/CoreFiles/Tuple.hpp Source/CoreFiles/TypeTraits.hpp Source/CoreFiles/Function.hpp Source/CoreFiles/Quaternion.hpp Source/CoreFiles/Quaternion.cpp Source/CoreFiles/Color.hpp Source/CoreFiles/Color.cpp Source/CoreFiles/EngineGlobals.hpp Source/Rendering/RenderWindow.hpp Source/Rendering/RenderWindow.cpp Source/CoreFiles/Delegate.hpp Source/Rendering/RenderPipeline.hpp Source/Rendering/RenderPipeline.cpp Source/CoreFiles/Assert.hpp Source/CoreFiles/Assert.cpp Source/CoreFiles/Optional.hpp Source/Rendering/RenderDevice.hpp Source/Rendering/RenderDevice.cpp Source/Rendering/VulkanCommon.hpp Source/Rendering/VulkanCommon.cpp Source/CoreFiles/Interface/IniConfig.hpp Source/CoreFiles/Interface/IniConfig.cpp Source/CoreFiles/Log.hpp Source/CoreFiles/Object/Object.hpp Source/CoreFiles/Object/Object.cpp Source/CoreFiles/Thread/Thread.hpp Source/CoreFiles/Thread/Thread.cpp Source/CoreFiles/EngineSettings.hpp Source/CoreFiles/EngineSettings.cpp Source/Rendering/RenderSwapChain.hpp Source/Rendering/RenderSwapChain.cpp Source/CoreFiles/EngineGlobals.cpp Source/CoreFiles/Thread/ThreadPool.hpp Source/CoreFiles/Thread/ThreadPool.cpp Source/CoreFiles/Future.hpp Source/CoreFiles/Future.cpp Source/CoreFiles/Actor/Actor.hpp Source/CoreFiles/Actor/Actor.cpp Source/CoreFiles/Math.cpp Source/CoreFiles/Memory.cpp Source/CoreFiles/Object/Allocator.hpp Source/CoreFiles/Object/Allocator.cpp Source/CoreFiles/Object/World.hpp Source/CoreFiles/Object/World.cpp Source/CoreFiles/Interface/Damageable.hpp Source/CoreFiles/Interface/Damageable.cpp Source/CoreFiles/Object/Level.hpp Source/CoreFiles/Object/Level.cpp Source/CoreFiles/Log.cpp)

add_custom_target(CompileShaders ALL COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/CompileShaders.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_dependencies(LiteEngine CompileShaders)

target_link_libraries(LiteEngine PRIVATE fmt)
target_link_libraries(LiteEngine PRIVATE glfw)
target_link_libraries(LiteEngine PRIVATE vulkan)
target_link_libraries(LiteEngine PRIVATE pthread)

target_compile_options(LiteEngine PUBLIC -std=c++20 -O3 -march=native -fvectorize -mno-vzeroupper -ffast-math -funsafe-math-optimizations -fno-rtti -mlong-double-128 -Wall -Wextra -Wpedantic -pedantic-errors -Wno-gnu-zero-variadic-macro-arguments)

