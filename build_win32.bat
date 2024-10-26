@echo off

set comp_flags=/nologo /std:c11 /Zi /Od /Wall
set link_flags=user32.lib
set root_dir=%cd%

mkdir build\win32 2>nul
pushd build\win32

cl %comp_flags% %root_dir%\build_win32.c /Fe"pxpat.exe" /link %link_flags%

if %errorlevel% neq 0 (
   echo Compilation failed.
   popd
   exit /b 1
)

popd
