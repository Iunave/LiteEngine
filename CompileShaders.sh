#!/bin/bash

cd Shaders/Source || return 1;

FragFiles='*.frag';
VertFiles='*.vert';

NumTotalFiles=$((${#FragFiles[@]} + ${#VertFiles[@]}));
CurrentFile=1;

function CompileFiles()
{
  local Name=$1;
  local Files=("${!Name}");
  local NumFiles=${#Files[@]};

  for((Index = 0; Index < NumFiles; ++Index));
  do
    local File=${Files[Index]};

    echo compiling $File ["$CurrentFile"/"$NumTotalFiles"];
    ((CurrentFile += 1));

    glslc $File -o ../Compiled/$File.spv;
  done
}

CompileFiles FragFiles;
CompileFiles VertFiles;
