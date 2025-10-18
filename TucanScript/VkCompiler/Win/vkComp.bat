@echo off
%VULKAN_SDK%\Bin\glslc.exe glsl.frag -o Out/frag.spv
%VULKAN_SDK%\Bin\glslc.exe glsl.vert -o Out/vert.spv
pause