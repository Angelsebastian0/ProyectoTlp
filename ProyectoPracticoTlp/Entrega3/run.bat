@echo off
setlocal enabledelayedexpansion

REM Script de apoyo para Windows XP/Dev-C++ 5
REM Compila el motor en modo consola (C++98) y luego lo ejecuta.

pushd %~dp0

set SRCDIR=src
set BINDIR=bin
set TARGET=%BINDIR%\motor_integration.exe
set GPP_EXE=g++

if not exist "%BINDIR%" mkdir "%BINDIR%"

REM Validar que g++ exista en el PATH (Dev-C++/MinGW)
where g++ >nul 2>nul
if errorlevel 1 (
    call :find_gpp
    if errorlevel 1 (
        popd
        exit /b 1
    )
)

echo Usando compilador: %GPP_EXE%

if not exist "%TARGET%" (
    echo Compilando...
    "%GPP_EXE%" -std=gnu++98 -Wall -Isrc -Ithird_party ^
        %SRCDIR%\integration_main.cpp ^
        %SRCDIR%\engine\api.cpp ^
        %SRCDIR%\interpreter\script_interpreter.cpp ^
        -o %TARGET%
    if errorlevel 1 (
        echo Error en la compilacion. Revise los mensajes anteriores.
        popd
        exit /b 1
    )
)

if "%~1"=="" (
    echo Ejecutando motor (elige el juego en el menu)...
    "%TARGET%"
) else (
    echo Ejecutando motor con script: %~1
    "%TARGET%" "%~1"
)

popd
endlocal
exit /b 0

:find_gpp
set "CANDIDATE=C:\Dev-Cpp\mingw32\bin\g++.exe"
if exist "%CANDIDATE%" (
    set "GPP_EXE=%CANDIDATE%"
    set "PATH=C:\Dev-Cpp\mingw32\bin;%PATH%"
    goto :eof
)

echo No se encontro g++ en PATH. Buscando g++.exe en %SystemDrive%\ (esto puede tardar)...
for /f "delims=" %%G in ('dir /b /s /a-d "%SystemDrive%\g++.exe" 2^>nul') do (
    set "GPP_EXE=%%G"
    for %%P in ("%%~dpG") do set "PATH=%%~P;%PATH%"
    goto :eof
)

echo No se pudo ubicar g++. Instala Dev-C++ 5 o agrega MinGW al PATH.
exit /b 1
