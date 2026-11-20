@echo off
rem Incremental build of one project (same environment as _build.bat).
rem   _build_proj.bat <project path under X:\trunk\xr_3da> <log file name>
rem   e.g. _build_proj.bat xrGame\xrGame.vcproj _build_game.log
setlocal
set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"
if exist "X:\SDK\dx2007" goto xready
if exist "X:\" goto fail
subst X: "%ROOT%"
:xready
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat" >nul 2>&1
set "DXNEW=C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)"
set "DX2007=X:\SDK\dx2007"
set "INCLUDE=%INCLUDE%;%DXNEW%\Include;%DX2007%\Include;X:\trunk;X:\trunk\xrCore;X:\SDK;X:\SDK\Xiph\Include;X:\SDK\MagicSoftware\FreeMagic\Include;X:\SDK\FreeImage"
set "LIB=%LIB%;%DXNEW%\Lib\x86;%DX2007%\Lib;X:\libraries;X:\SDK\FreeImage;X:\SDK\Xiph\Libs;X:\SDK\MagicSoftware\FreeMagic\Library\ReleaseDLL"
set LINK="%DX2007%\Lib\dxguid.lib"
set CL=/D_WIN32_IE=0x0500
vcbuild /useenv "X:\trunk\xr_3da\%~1" "Release|Win32" > "%ROOT%\%~2" 2>&1
echo done >> "%ROOT%\%~2"
goto end
:fail
echo X: busy > "%ROOT%\%~2"
:end
endlocal
