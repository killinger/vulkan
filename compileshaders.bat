@echo off

SETLOCAL
set OutputDirectory=vulkangame\

cls
pushd %OutputDirectory%
%VULKAN_SDK%/bin32/glslangValidator.exe -V shader.vert
%VULKAN_SDK%/bin32/glslangValidator.exe -V shader.frag
popd