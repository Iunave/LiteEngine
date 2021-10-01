#!/bin/bash
cd Shaders/Source;

FragFiles=*.frag;
VertFiles=*.vert;

for File in $FragFiles;
do 
    glslc $File -o ../Compiled/$File.spv &
done

for File in $VertFiles;
do 
    glslc $File -o ../Compiled/$File.spv &
done
