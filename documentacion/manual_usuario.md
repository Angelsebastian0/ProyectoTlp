# Manual de Usuario

## 🎮 Propósito
Este manual explica cómo compilar, ejecutar y probar el proyecto del motor de juegos desarrollado en C++.

## ⚙️ Instalación y Ejecución

### 1. Requisitos Previos
- Tener instalado un compilador C++ (g++ o similar).  
- Tener Visual Studio Code o cualquier IDE compatible.

### 2. Compilación
1. Abrir una terminal en la carpeta de la entrega deseada, por ejemplo:
   ```bash
   cd ProyectoPracticoTlp/Entrega2
   ```
2. Compilar el código fuente:
   ```bash
   g++ main.cpp -o motor
   ```
3. Ejecutar el programa:
   ```bash
   ./motor
   ```

### 3. Archivos de Prueba
- Los archivos `.brik` contienen el código fuente del mini-lenguaje.  
- Ejemplo: `mini-lenguaje.brik` define las reglas básicas de un juego tipo *snake* o *tetris*.

### 4. Comportamiento Esperado
- Se abrirá una ventana de 640x480 píxeles.  
- El motor dibuja elementos gráficos básicos (bloques, texto, etc.).  
- Las teclas del teclado controlan el movimiento de los objetos renderizados.

## 🧩 Próximas Funcionalidades (Entrega 3)
- Integración completa entre analizador y motor.  
- Ejecución dinámica de distintos juegos definidos por el usuario.  
- Módulo de interpretación del archivo `.brik`.

---
> Si ocurre algún error durante la ejecución, verificar el archivo de entrada o el formato de los tokens generados.
