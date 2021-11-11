#!/bin/bash

cd Binaries/Release || return 1;

Executables='*.out';

for Executable in $Executables;
do
  echo stripping debug info for $Executable;
  strip --strip-all $Executable;
done