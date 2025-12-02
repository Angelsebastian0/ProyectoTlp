# Manual Técnico

## 🧠 Propósito del Proyecto
El objetivo es desarrollar un **motor de juegos modular** que ejecute juegos definidos mediante un lenguaje propio.  
La arquitectura se compone de tres etapas progresivas: análisis léxico/sintáctico, motor de juego y lógica integrada.

## 🏗️ Arquitectura General
El sistema está dividido en tres componentes principales:

1. **Analizador de Lenguaje (Entrega 1):**
   - Procesa un archivo `.brik` con las reglas del juego.
   - Incluye un analizador léxico que convierte el texto en tokens.
   - Usa un analizador sintáctico que construye el Árbol de Sintaxis Abstracta (AST).
   - Gestiona una tabla de símbolos para los identificadores del lenguaje.

2. **Motor de Juego (Entrega 2):**
   - Bucle principal que gestiona eventos, actualizaciones y estado interno.
   - Control básico de entradas por teclado.
   - En la versión actual (para XP), el dibujo se hace en consola de texto.

3. **Integración y Lógica del Juego (Entrega 3):**
   - Integra el analizador y el motor.
   - El motor lee scripts simples en texto (`.script`) sin JSON ni SDL.
   - Permite ejecutar juegos distintos sin recompilar el motor.

## 🧩 Componentes Clave
- **GramaticaEBNF.txt:** define la estructura formal del mini-lenguaje.  
- **mini-lenguaje.brik:** contiene un ejemplo del lenguaje para pruebas.  
- **tokens.txt:** salida del analizador léxico.  
- **main.cpp (Entrega1 y 2):** archivos fuente principales del proyecto.

## 💾 Requisitos de Sistema
- Windows XP o posterior
- Procesador AMD Athlon XP o superior
- 512 MB RAM mínimo
- Compilador compatible con C++98/C++03 (Dev-C++ 5 con MinGW funciona)

## 🧰 Herramientas Recomendadas
- **Visual Studio Code**
- **G++ / MinGW**
- **Makefile (opcional)**

---
> El código está documentado internamente con comentarios explicativos sobre cada módulo y función principal.

## 🛠️ Guía rápida para compilar en Windows XP con Dev-C++ 5
- El código ya está en modo consola y C++98/C++03, sin SDL ni JSON.
- Abre `ProyectoPracticoTlp/Entrega3` en Dev-C++ 5, configura el estándar a `-std=gnu++98` y compila `integration_main.cpp` junto con `engine/api.cpp` y `interpreter/script_interpreter.cpp`.
- No se requieren DLLs externas; el binario cabe en un disquete de 1.44 MB si se compila con `-Os -s`.
