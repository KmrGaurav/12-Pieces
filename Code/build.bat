@echo off
pushd ..\build
cl -FC -Zi ..\Code\Win32_Baratini.cpp user32.lib gdi32.lib
popd