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

2. **Motor Gráfico y de Juego (Entrega 2):**
   - Crea una ventana de 640x480 píxeles.
   - Contiene un bucle principal que gestiona eventos, actualizaciones y renderizado.
   - Implementa funciones gráficas básicas para dibujar bloques, texto y puntuación.
   - Incluye un módulo de control de entradas del teclado.

3. **Integración y Lógica del Juego (Entrega 3 - futura):**
   - Integra el analizador y el motor.
   - El motor leerá reglas del lenguaje desde archivos `.brik`.
   - Permitirá ejecutar juegos distintos sin recompilar el motor.

## 🧩 Componentes Clave
- **GramaticaEBNF.txt:** define la estructura formal del mini-lenguaje.  
- **mini-lenguaje.brik:** contiene un ejemplo del lenguaje para pruebas.  
- **tokens.txt:** salida del analizador léxico.  
- **main.cpp (Entrega1 y 2):** archivos fuente principales del proyecto.

## 💾 Requisitos de Sistema
- Windows XP o posterior  
- Procesador AMD Athlon XP o superior  
- 512 MB RAM mínimo  
- Compilador compatible con C++11 o superior

## 🧰 Herramientas Recomendadas
- **Visual Studio Code**
- **G++ / MinGW**
- **Makefile (opcional)**

---
> El código está documentado internamente con comentarios explicativos sobre cada módulo y función principal.
