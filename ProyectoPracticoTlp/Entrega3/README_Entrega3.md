# Entrega 3 - Integración (Motor + Analizador)

## Requisitos
- g++ compatible con C++98/C++03 (Dev-C++ 5 con MinGW funciona)
- Consola de texto (no se requiere SDL ni JSON)

## Compilar
```bash
make
```

### En Windows (CMD)
- Asegúrate de tener g++ de MinGW/Dev-C++ 5 en el PATH.
- Ejecuta `run.bat` desde la carpeta `Entrega3` (compila automáticamente si falta el ejecutable).
- Si no lo encuentra, el script intentará añadir `C:\Dev-Cpp\mingw32\bin` y buscar `g++.exe` en todo el disco del sistema.
- Puedes pasar directamente el script a ejecutar: `run.bat games\snake.script`.

## Ejecutar
```bash
./bin/motor_integration games/snake.script
```
O sin argumentos para ver el menú y escoger Tetris o Snake.
