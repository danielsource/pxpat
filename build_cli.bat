@echo off

set comp_flags=/nologo /std:c11 /Zi /Od /Wall /D_CRT_SECURE_NO_WARNINGS /wd5045 /wd4820
set link_flags=
set root_dir=%cd%

mkdir build\cli 2>nul
pushd build\cli

cl %comp_flags% %root_dir%\build_cli.c /Fe"pxpat.exe" /link %link_flags%

if %errorlevel% neq 0 (
   echo Compilation failed.
   popd
   exit /b 1
)

popd
