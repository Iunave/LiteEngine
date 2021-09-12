cmake_minimum_required(VERSION 3.16)

project(LiteEngine VERSION 1.0)
add_compile_definitions(PROJECT_NAME="LiteEngine")

set(CompilerPath=/home/user/Programs/llvm-project-llvmorg-12.0.0/build/bin/clang++)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER ${CompilerPath})

include_directories(Source/CoreFiles)

set(CMAKE_UNITY_BUILD 0)
add_compile_definitions(UNITY_BUILD=false)

add_subdirectory(Source/Rendering/glfw-3.3.3)
add_subdirectory(Source/Format)

set(CoreFilesSrc
    Source/CoreFiles/GuardedMain.cpp
    Source/CoreFiles/Array.cpp
    Source/CoreFiles/Assert.cpp
    Source/CoreFiles/Atomic.cpp
    Source/CoreFiles/Assert.cpp
    Source/CoreFiles/Color.cpp
    Source/CoreFiles/Definitions.cpp
    Source/CoreFiles/EngineGlobals.cpp
    Source/CoreFiles/EngineSettings.cpp
    Source/CoreFiles/Delegate.cpp
    Source/CoreFiles/Function.cpp
    Source/CoreFiles/Future.cpp
    Source/CoreFiles/Log.cpp
    Source/CoreFiles/Math.cpp
    Source/CoreFiles/Memory.cpp
    Source/CoreFiles/Optional.cpp
    Source/CoreFiles/Quaternion.cpp
    Source/CoreFiles/Simd.cpp
    Source/CoreFiles/SmartPointer.cpp
    Source/CoreFiles/String.cpp
    Source/CoreFiles/Tuple.cpp
    Source/CoreFiles/TypeTraits.cpp
    Source/CoreFiles/Vector.cpp
)

set(ObjectFilesSrc
    Source/CoreFiles/Object/Allocator.cpp
    Source/CoreFiles/Object/Level.cpp
    Source/CoreFiles/Object/Object.cpp
    Source/CoreFiles/Object/World.cpp
)

set(ThreadFilesSrc
    Source/CoreFiles/Thread/Thread.cpp
    Source/CoreFiles/Thread/ThreadPool.cpp
)

set(ActorFilesSrc
    Source/CoreFiles/Actor/Actor.cpp
    Source/CoreFiles/Actor/Robot.cpp
    Source/CoreFiles/Actor/BuildComponent/BuildComponent.cpp
    Source/CoreFiles/Actor/BuildComponent/WeaponComponent.cpp
    Source/CoreFiles/Actor/BuildComponent/MovementComponent.cpp
)

set(InterfaceFilesSrc
        Source/CoreFiles/Interface/Damageable.cpp
        Source/CoreFiles/Interface/IniConfig.cpp
        Source/CoreFiles/Interface/Tick.cpp
)

set(RenderFilesSrc
    Source/Rendering/VulkanRenderer.cpp
    Source/Rendering/RenderWindow.cpp
)

set(GeneralCompilerFlags
        -std=c++20
        -O3
        -mfma
        -msse
        -msse2
        -msse3
        -msse4.1
        -msse4.2
        -fvectorize
        -mno-vzeroupper
        -ffast-math
        -funsafe-math-optimizations
        -fno-rtti
        -mlong-double-128
        -Wall
        -Wextra
        -Wpedantic
        -pedantic-errors
        -Wno-gnu-zero-variadic-macro-arguments
)

add_compile_definitions(DEBUG=true)

add_executable(LiteEngine Source/CoreFiles/ChooseExecutableMain.cpp)
target_compile_options(LiteEngine PRIVATE ${GeneralCompilerFlags})

add_executable(LiteEngine_avx2 ${CoreFilesSrc} ${ObjectFilesSrc} ${ThreadFilesSrc} ${ActorFilesSrc} ${InterfaceFilesSrc} ${RenderFilesSrc})
target_compile_options(LiteEngine_avx2 PRIVATE ${GeneralCompilerFlags} -mavx -mavx2)
add_dependencies(LiteEngine LiteEngine_avx2)

#target_link_libraries(LiteEngine_avx2 PRIVATE ubsan)
target_link_libraries(LiteEngine_avx2 PRIVATE fmt)
target_link_libraries(LiteEngine_avx2 PRIVATE glfw)
target_link_libraries(LiteEngine_avx2 PRIVATE vulkan)
target_link_libraries(LiteEngine_avx2 PRIVATE pthread)

add_executable(LiteEngine_avx512 ${CoreFilesSrc} ${ObjectFilesSrc} ${ThreadFilesSrc} ${ActorFilesSrc} ${InterfaceFilesSrc} ${RenderFilesSrc})
target_compile_options(LiteEngine_avx512 PRIVATE ${GeneralCompilerFlags} -mavx -mavx2 -mavx512f -mavx512dq -mavx512bw)
add_dependencies(LiteEngine LiteEngine_avx512)

target_link_libraries(LiteEngine_avx512 PRIVATE fmt)
target_link_libraries(LiteEngine_avx512 PRIVATE glfw)
target_link_libraries(LiteEngine_avx512 PRIVATE vulkan)
target_link_libraries(LiteEngine_avx512 PRIVATE pthread)

add_custom_target(CompileShaders ALL COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/CompileShaders.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(LiteEngine CompileShaders)