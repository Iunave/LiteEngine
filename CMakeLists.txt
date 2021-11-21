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
    Source/CoreFiles/Time.cpp
    Source/CoreFiles/Iterator.cpp
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
    Source/Rendering/RenderSettings.cpp
    Source/Rendering/RenderWindow.cpp
    Source/Rendering/Vertex.cpp
)

set(LauncherFilesSrc
    Source/CoreFiles/ChooseMain.cpp
    Source/CoreFiles/String.cpp
    Source/CoreFiles/Log.cpp
    Source/CoreFiles/Thread/Thread.cpp
)

set(GeneralCompilerFlags
    -std=c++20
    -O3
    -fvectorize
    -mno-vzeroupper
    -ffast-math
    -funroll-loops
    -funsafe-math-optimizations
    #-fno-rtti
    -fno-exceptions
    -mlong-double-128
    -Wall
    -Wextra
    -Wpedantic
    -pedantic-errors
    -Wno-gnu-zero-variadic-macro-arguments
    -Wno-zero-length-array
)

set(CMAKE_PROJECT_NAME LiteEngine)
set(CMAKE_ENGINE_NAME LiteEngine)
set(CMAKE_ENGINE_VERSION 1)

set(LauncherExecutableName ${CMAKE_PROJECT_NAME}.out)

add_executable(${LauncherExecutableName} ${LauncherFilesSrc})

target_compile_definitions(${LauncherExecutableName} PRIVATE DEBUG=false)
target_compile_definitions(${LauncherExecutableName} PRIVATE PROJECT_NAME="${CMAKE_PROJECT_NAME}")
target_compile_definitions(${LauncherExecutableName} PRIVATE ENGINE_NAME="${CMAKE_ENGINE_NAME}")
target_compile_definitions(${LauncherExecutableName} PRIVATE ENGINE_VERSION=${CMAKE_ENGINE_VERSION})
target_compile_definitions(${LauncherExecutableName} PRIVATE SOURCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_options(${LauncherExecutableName} PRIVATE ${GeneralCompilerFlags} -march=skylake)
target_link_libraries(${LauncherExecutableName} PUBLIC fmt)
target_link_libraries(${LauncherExecutableName} PUBLIC pthread)

#movie target
add_executable(Movies.out ${CoreFilesSrc} ${ObjectFilesSrc} ${ThreadFilesSrc})

target_compile_definitions(Movies.out PRIVATE DEBUG=false)
target_compile_definitions(Movies.out PRIVATE PROJECT_NAME="${CMAKE_PROJECT_NAME}")
target_compile_definitions(Movies.out PRIVATE ENGINE_NAME="${CMAKE_ENGINE_NAME}")
target_compile_definitions(Movies.out PRIVATE ENGINE_VERSION=${CMAKE_ENGINE_VERSION})
target_compile_definitions(Movies.out PRIVATE SOURCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_options(Movies.out PRIVATE ${GeneralCompilerFlags} -march=skylake)
target_link_libraries(Movies.out PUBLIC fmt)
target_link_libraries(Movies.out PUBLIC pthread)

#movie target

add_custom_command(
        TARGET ${LauncherExecutableName}
        PRE_BUILD
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/CompileShaders.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "compiling shaders...")

add_custom_command(
        TARGET ${LauncherExecutableName}
        POST_BUILD
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/StripDebugInfo.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "stripping debug info...")

set(ExecutableName)
set(CompileOptions)

macro(MakeExecutable Version CpuTarget)

    set(ExecutableName "${CMAKE_PROJECT_NAME}_${CpuTarget}_${Version}.out")

    add_executable(${ExecutableName} ${CoreFilesSrc} ${ObjectFilesSrc} ${ThreadFilesSrc} ${ActorFilesSrc} ${InterfaceFilesSrc} ${RenderFilesSrc})

    if(${Version} STREQUAL "debug")
        set(CompileOptions ${GeneralCompilerFlags} -g -march=${CpuTarget})
        target_compile_definitions(${ExecutableName} PRIVATE DEBUG=true)
    else(${Version} STREQUAL "release")
        set(CompileOptions ${GeneralCompilerFlags} -march=${CpuTarget})
        target_compile_definitions(${ExecutableName} PRIVATE DEBUG=false)
    endif()

    target_compile_options(${ExecutableName} PRIVATE ${CompileOptions})

    target_compile_definitions(${ExecutableName} PRIVATE EXECUTABLE_NAME="${ExecutableName}")
    target_compile_definitions(${ExecutableName} PRIVATE PROJECT_NAME="${CMAKE_PROJECT_NAME}")
    target_compile_definitions(${ExecutableName} PRIVATE ENGINE_NAME="${CMAKE_ENGINE_NAME}")
    target_compile_definitions(${ExecutableName} PRIVATE ENGINE_VERSION=${CMAKE_ENGINE_VERSION})
    target_compile_definitions(${ExecutableName} PRIVATE SOURCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}")

    target_link_libraries(${ExecutableName} PUBLIC fmt)
    target_link_libraries(${ExecutableName} PUBLIC glfw)
    target_link_libraries(${ExecutableName} PUBLIC vulkan)
    target_link_libraries(${ExecutableName} PUBLIC pthread)

    add_custom_command(
            TARGET ${ExecutableName}
            PRE_BUILD
            COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/CompileShaders.sh
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "compiling shaders...")

    add_custom_command(
            TARGET ${ExecutableName}
            POST_BUILD
            COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/StripDebugInfo.sh
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "stripping debug info...")

    add_dependencies(${LauncherExecutableName} ${ExecutableName})

endmacro(MakeExecutable)

MakeExecutable(release skylake)
MakeExecutable(release skylake-avx512)
MakeExecutable(release cannonlake)
MakeExecutable(release cascadelake)
MakeExecutable(release tigerlake)
MakeExecutable(release cooperlake)
MakeExecutable(release alderlake)
MakeExecutable(release znver1)
MakeExecutable(release znver2)
MakeExecutable(release znver3)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
MakeExecutable(debug skylake)
MakeExecutable(debug skylake-avx512)
MakeExecutable(debug cannonlake)
MakeExecutable(debug cascadelake)
MakeExecutable(debug tigerlake)
MakeExecutable(debug cooperlake)
MakeExecutable(debug alderlake)
MakeExecutable(debug znver1)
MakeExecutable(debug znver2)
MakeExecutable(debug znver3)
endif()
