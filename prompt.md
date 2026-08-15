ACTÚA COMO UN INGENIERO DE SOFTWARE EMBEBIDO SENIOR.

REQUERIMIENTO PRINCIPAL:
Desarrollar un driver completo en C++ para un display LCD 20x04 (Hitachi HD44780) controlado mediante un expansor I2C PCF8574, conectado a una Raspberry Pi (Linux/ARM).

ESTRUCTURA DE DIRECTORIOS Y ARCHIVOS OBLIGATORIA:
- src/main.cpp
- src/lcd.cpp
- include/lcd.hpp
- bin/App (ejecutable final)
- obj/*.o (archivos objeto)
- docs/ (carpeta con documentación interna)
- Makefile (compilación profesional)
- README.md (con instrucciones de uso y compilación)

REGLAS DE CALIDAD Y ESTILO (CÓDIGO ROBUSTO):
1. Código en C++17 o superior.
2. Uso de clases y encapsulamiento (RAII, manejo de recursos).
3. Manejo de errores con excepciones o códigos de retorno claros.
4. Comentarios DOXYGEN en TODOS los métodos públicos y privados.
5. Separación clara entre interfaz (header) e implementación (cpp).
6. Inicialización de pines y configuración I2C correcta (dirección 0x27 o 0x3F).
7. Funciones para: init, clear, home, writeChar, writeString, setCursor, createChar (CGROM).
8. Uso de sleeps/delays precisos para timings del HD44780 (usleep o nanosleep).
9. Compilación con flags: -Wall -Wextra -Werror -pedantic -std=c++17.
10. Makefile con reglas: all, clean, distclean, debug, release.
11. README.md con: descripción, dependencias (i2c-dev, wiringPi o libi2c), instrucciones de compilación y ejecución.
12. docs/DESIGN.md con diagrama de flujo y explicación de la máquina de estados del LCD.

ENTREGA ESPERADA:
- El contenido completo de TODOS los archivos mencionados.
- El código debe ser funcional, compilable y ejecutable en Raspberry Pi OS.
- Incluir un ejemplo en main.cpp que muestre un mensaje tipo "Hola Mundo" y un contador en la segunda línea.

FORMATO DE SALIDA:
- Cada archivo debe ir precedido por su ruta y nombre, entre bloques de código con lenguaje "cpp" o "makefile".
- Ejemplo:
  ### src/main.cpp
  ```cpp
  // contenido aquí
