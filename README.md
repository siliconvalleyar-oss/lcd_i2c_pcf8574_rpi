# LCD HD44780 PCF8574 Driver

Driver completo en C++17 para displays LCD 20x04 (HD44780) controlados mediante un expansor I2C PCF8574, desarrollado para Raspberry Pi (Linux/ARM).

## Descripcion

Este driver implementa una interfaz completa para controlar displays alfanumericos HD44780 en modo de 4 bits a traves de un adaptador I2C basado en el PCF8574. Cumple con los estandares de codigo robusto, manejo de errores y documentacion DOXYGEN.

## Dependencias

- Sistema operativo: Raspberry Pi OS o cualquier distribucion Linux con soporte I2C
- Modulo kernel: `i2c-dev`
- Compilador: `g++` con soporte C++17
- Herramientas: `make`

### Habilitar I2C en Raspberry Pi

```bash
sudo raspi-config
```

Navegar a `Interfacing Options` -> `I2C` y habilitarlo. Reiniciar el sistema.

Verificar que el modulo esta cargado:

```bash
lsmod | grep i2c_dev
```

Si no esta cargado:

```bash
sudo modprobe i2c-dev
```

## Conexionado

```
PCF8574    HD44780 LCD
-------    -----------
P0 (D4)  -> D4
P1 (D5)  -> D5
P2 (D6)  -> D6
P3 (D7)  -> D7
P4       -> RS
P5       -> RW (conectar a GND en hardware si no se quiere controlar escritura)
P6       -> EN
P7       -> Backlight
SDA      -> SDA (GPIO2, pin 3)
SCL      -> SCL (GPIO3, pin 5)
VCC      -> 5V
GND      -> GND
```

## Direcciones I2C

Las direcciones mas comunes del PCF8574 son:
- `0x27` - Mas comun en backpacks con jumper A0-A2 sin conectar (todos en GND)
- `0x3F` - Algunos backpacks con A0-A2 conectados a VCC

Para detectar la direccion del dispositivo:

```bash
sudo i2cdetect -y 1
```

## Compilacion

### Modo Release (optimizado)

```bash
make release
```

El ejecutable se genera en `bin/App`.

### Modo Debug (con simbolos de depuracion)

```bash
make debug
```

### Limpiar archivos objeto

```bash
make clean
```

### Limpiar todo (incluyendo ejecutable)

```bash
make distclean
```

### Compilacion manual

```bash
g++ -Wall -Wextra -Werror -pedantic -std=c++17 -Iinclude -c src/lcd.cpp -o obj/lcd.o
g++ -Wall -Wextra -Werror -pedantic -std=c++17 -Iinclude -c src/main.cpp -o obj/main.o
g++ obj/lcd.o obj/main.o -o bin/App
```

## Ejecucion

```bash
sudo ./bin/App
```

El programa requiere privilegios de root o pertenecer al grupo `i2c` para acceder al bus I2C.

Para ejecutar sin sudo (recomendado para desarrollo):

```bash
sudo usermod -aG i2c $USER
```

Luego cerrar sesion y volver a ingresar.

## API del Driver

### Constructores

```cpp
LCD lcd("/dev/i2c-1", 0x27);
```

### Inicializacion

```cpp
lcd.init();
lcd.backlightOn();
lcd.clear();
lcd.home();
```

### Escritura

```cpp
lcd.writeChar('A');
lcd.writeString("Hola Mundo");
```

### Posicion del cursor

```cpp
lcd.setCursor(0, 0); // Columna 0, Fila 0
lcd.setCursor(19, 3); // Columna 19, Fila 3 (ultima posicion en display 20x4)
```

### Control del display

```cpp
lcd.displayOn();
lcd.displayOff();
lcd.cursorOn();
lcd.cursorOff();
lcd.blinkOn();
lcd.blinkOff();
lcd.backlightOn();
lcd.backlightOff();
```

### Caracteres personalizados (CGROM)

```cpp
uint8_t heart[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000
};
lcd.createChar(0, heart);
lcd.setCursor(0, 0);
lcd.writeChar(0);
```

### Limpiar y home

```cpp
lcd.clear();    // Borra el display y retorna el cursor a (0,0)
lcd.home();     // Retorna el cursor a (0,0) sin borrar
```

## Estructura del Proyecto

```
.
├── include/
│   └── lcd.hpp
├── src/
│   ├── lcd.cpp
│   └── main.cpp
├── bin/
│   └── App
├── obj/
│   └── *.o
├── docs/
│   └── DESIGN.md
├── Makefile
└── README.md
```

## Solucion de Problemas

### Permission denied al acceder a /dev/i2c-1

Verificar permisos:

```bash
ls -l /dev/i2c-*
```

Debe pertenecer al grupo `i2c`:

```bash
sudo usermod -aG i2c $USER
```

### No se detecta el dispositivo I2C

Verificar conexiones fisicas y direccion:

```bash
sudo i2cdetect -y 1
```

Si no aparece ningun dispositivo, revisar:
- Conexiones SDA/SCL
- Alimentacion (5V y GND)
- Direccion I2C del modulo

### El display no se inicializa

- Verificar que el modulo PCF8574 tenga el jumper de contraste ajustado correctamente (potenciometro)
- Asegurar que la direccion I2C sea correcta
- Verificar que el bus I2C este habilitado

## Licencia

MIT
