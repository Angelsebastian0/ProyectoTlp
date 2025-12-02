@echo off
setlocal enabledelayedexpansion

REM Script de apoyo para Windows XP/Dev-C++ 5
REM Compila el motor en modo consola (C++98) y luego lo ejecuta.

pushd %~dp0

set SRCDIR=src
set BINDIR=bin
set TARGET=%BINDIR%\motor_integration.exe

if not exist "%BINDIR%" mkdir "%BINDIR%"

REM Validar que g++ exista en el PATH (Dev-C++/MinGW)
where g++ >nul 2>nul
if errorlevel 1 (
    echo No se encontro g++. Anada la carpeta bin de MinGW/Dev-C++ al PATH e intente de nuevo.
    popd
    exit /b 1
)

if not exist "%TARGET%" (
    echo Compilando...
    g++ -std=gnu++98 -Wall -Isrc -Ithird_party ^
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
