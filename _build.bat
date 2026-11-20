@echo off
rem ==============================================================
rem  Build script: Dead, DeZoWaRe
rem
rem
rem  Requirements:
rem    - Visual Studio 2008 SP1
rem    - DirectX SDK (June 2010) in its default location
rem
rem  Run this file from the folder that contains trunk and SDK.
rem ==============================================================
setlocal

set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"

rem --- drive X: is mandatory: x:\binaries is hardcoded in the projects
if exist "X:\SDK\dx2007" goto xready
if exist "X:\" goto xbusy
subst X: "%ROOT%"
if errorlevel 1 goto xfail
echo [+] Drive X: mounted on %ROOT%
goto xready

:xbusy
echo [!] Drive X: is already mapped to another folder.
echo [!] Free it first:  subst X: /D
goto fail

:xfail
echo [!] Could not create drive X:
goto fail

:xready

set "VSVARS=C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"
if not exist "%VSVARS%" goto novs
call "%VSVARS%" >nul 2>&1
echo [+] Visual Studio 2008 environment ready

set "DXNEW=C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)"
if not exist "%DXNEW%\Include\d3dx9.h" goto nodx
echo [+] DirectX SDK June 2010 found

rem --- DirectPlay and dxerr9 were dropped from June 2010; shipped in SDK\dx2007
set "DX2007=X:\SDK\dx2007"

set "INCLUDE=%INCLUDE%;%DXNEW%\Include;%DX2007%\Include;X:\trunk;X:\trunk\xrCore;X:\SDK;X:\SDK\Xiph\Include;X:\SDK\MagicSoftware\FreeMagic\Include;X:\SDK\FreeImage"
set "LIB=%LIB%;%DXNEW%\Lib\x86;%DX2007%\Lib;X:\libraries;X:\SDK\FreeImage;X:\SDK\Xiph\Libs;X:\SDK\MagicSoftware\FreeMagic\Library\ReleaseDLL"

rem --- DirectPlay GUIDs exist only in dxguid.lib from the 2007 SDK
set LINK="%DX2007%\Lib\dxguid.lib"
rem --- MFC in stalker_net needs _WIN32_IE consistent with _WIN32_WINNT
set CL=/D_WIN32_IE=0x0500

echo.
echo === Building. This takes 20-60 minutes. ===
echo.
vcbuild /useenv "X:\trunk\xr_3da\XR_3DA.sln" "Release|Win32"

echo.
echo === Output: X:\binaries ===
dir /b X:\binaries\*.exe X:\binaries\*.dll 2>nul
goto end

:novs
echo [!] Visual Studio 2008 not found:
echo     %VSVARS%
goto fail

:nodx
echo [!] DirectX SDK June 2010 not found:
echo     %DXNEW%
goto fail

:fail
echo.
echo Build was not started.
endlocal
exit /b 1

:end
endlocal
