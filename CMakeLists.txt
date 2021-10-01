cmake_minimum_required(VERSION 3.16)

project(LiteEngine VERSION 1.0)

set(CompilerPath /home/user/Programs/llvm-project-llvmorg-12.0.0/build/bin/clang++)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER ${CompilerPath})

include_directories(Source/CoreFiles)
include_directories(Source)

set(CMAKE_UNITY_BUILD 0)
add_compile_definitions(UNITY_BUILD=${CMAKE_UNITY_BUILD})

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
    -fno-exceptions
    -mlong-double-128
    -Wall
    -Wextra
    -Wpedantic
    -pedantic-errors
    -Wno-gnu-zero-variadic-macro-arguments
)

set(CMAKE_PROJECT_NAME LiteEngine)
set(CMAKE_ENGINE_NAME LiteEngine)
set(CMAKE_ENGINE_VERSION 1)

set(LauncherExecutableName "${CMAKE_PROJECT_NAME}_launcher")

#arguments for ${LauncherExecutableName} are: debug = run debug version, avx256 = run avx256 version, avx512 = run avx512 version, benchmark=N = run program N times
add_executable(${LauncherExecutableName} Source/CoreFiles/ChooseExecutableMain.cpp)

target_compile_options(${LauncherExecutableName} PRIVATE ${GeneralCompilerFlags})

target_compile_definitions(${LauncherExecutableName} PRIVATE PROJECT_NAME="${CMAKE_PROJECT_NAME}")
target_compile_definitions(${LauncherExecutableName} PRIVATE ENGINE_NAME="${CMAKE_ENGINE_NAME}")
target_compile_definitions(${LauncherExecutableName} PRIVATE ENGINE_VERSION=${CMAKE_ENGINE_VERSION})

add_custom_target(CompileShaders ALL COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/CompileShaders.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_dependencies(${LauncherExecutableName} CompileShaders)

set(ExecutableName)
set(CompileOptions)

macro(MakeExecutable bDebug avx_version)

    if(${bDebug})
        if("${avx_version}" STREQUAL "avx256")

            set(ExecutableName "${CMAKE_PROJECT_NAME}_avx256_debug")
            set(CompileOptions ${GeneralCompilerFlags} -mavx -mavx2)

        elseif("${avx_version}" STREQUAL "avx512")

            set(ExecutableName "${CMAKE_PROJECT_NAME}_avx512_debug")
            set(CompileOptions ${GeneralCompilerFlags} -mavx -mavx2 -mavx512f -mavx512dq -mavx512bw)

        endif()
    else()
        if("${avx_version}" STREQUAL "avx256")

            set(ExecutableName "${CMAKE_PROJECT_NAME}_avx256")
            set(CompileOptions ${GeneralCompilerFlags} -mavx -mavx2 -s)

        elseif("${avx_version}" STREQUAL "avx512")

            set(ExecutableName "${CMAKE_PROJECT_NAME}_avx512")
            set(CompileOptions ${GeneralCompilerFlags} -mavx -mavx2 -mavx512f -mavx512dq -mavx512bw -s)

        endif()
    endif()

    add_executable(${ExecutableName} ${CoreFilesSrc} ${ObjectFilesSrc} ${ThreadFilesSrc} ${ActorFilesSrc} ${InterfaceFilesSrc} ${RenderFilesSrc})

    target_compile_options(${ExecutableName} PRIVATE ${CompileOptions})

    target_compile_definitions(${ExecutableName} PRIVATE EXECUTABLE_NAME="${ExecutableName}")
    target_compile_definitions(${ExecutableName} PRIVATE DEBUG=${bDebug})
    target_compile_definitions(${ExecutableName} PRIVATE PROJECT_NAME="${CMAKE_PROJECT_NAME}")
    target_compile_definitions(${ExecutableName} PRIVATE ENGINE_NAME="${CMAKE_ENGINE_NAME}")
    target_compile_definitions(${ExecutableName} PRIVATE ENGINE_VERSION=${CMAKE_ENGINE_VERSION})

    target_link_libraries(${ExecutableName} PRIVATE fmt)
    target_link_libraries(${ExecutableName} PRIVATE glfw)
    target_link_libraries(${ExecutableName} PRIVATE vulkan)
    target_link_libraries(${ExecutableName} PRIVATE pthread)

    add_dependencies(${LauncherExecutableName} ${ExecutableName})

endmacro(MakeExecutable)

MakeExecutable(false avx256)
MakeExecutable(false avx512)
MakeExecutable(true avx256)
MakeExecutable(true avx512)