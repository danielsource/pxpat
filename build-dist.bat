@echo off

set comp_flags=/nologo /std:c11 /Wall /DNDEBUG
set link_flags=
set root_dir=%cd%

del /s /q build
mkdir build 2>nul
pushd build

cl %comp_flags% %root_dir%\pxpat.c /Fe"pxpat.exe" /link %link_flags%

if %errorlevel% neq 0 (
   echo Compilation failed.
   popd
   exit /b 1
)

popd
