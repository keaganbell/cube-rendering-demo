@echo off

setlocal

set name=render_demo

if "%~1"=="clean" (
    del %name%.exe
    del %name%.obj
    del %name%.pdb
)

set incs=-I%~dp0ext\inc
set libs=user32.lib gdi32.lib opengl32.lib
set exe_defines=-DAPP_DLL_NAME="\"%name%.dll\""

if not exist build (mkdir build)
pushd build

where /q cl (
    cl -FC -W4 -Zi -Od -MT -nologo %exe_defines% %incs% ..\src\windows_rendering.cpp -Fdcompiler_%name%.pdb -Fe%name%.exe -link -INCREMENTAL:NO %libs% -PDB:linker_%name%.pdb
    cl -FC -W4 -Zi -Od -MT -nologo %incs% ..\src\rendering.cpp -Fdcompiler_dll_%name%.pdb -LD -link -OUT:%name%.dll -INCREMENTAL:NO %libs% /EXPORT:Initialize /EXPORT:UpdateAndRender -PDB:linker_dll_%name%.pdb
)
popd

endlocal