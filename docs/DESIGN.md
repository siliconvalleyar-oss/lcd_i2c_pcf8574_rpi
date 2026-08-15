# Design Document - LCD HD44780 PCF8574 Driver

## Hardware Architecture

```
+----------------+         I2C (SDA/SCL)        +----------------+
|                |------------------------------>|                |
|  Raspberry Pi  |                               |    PCF8574     |
|   (I2C Master) |<------------------------------|  (I2C Slave)   |
|                |                               |                |
+----------------+                               +-------+--------+
                                                       |
                                          +------------+------------+
                                          | P0 | P1 | P2 | P3 | ... |
                                          | D4 | D5 | D6 | D7 | P7 |
                                          +----+----+----+----+-----+
                                                   |
                                                   v
                                         +-------------------+
                                         |   HD44780 LCD     |
                                         |   20x04 Display   |
                                         +-------------------+
```

### PCF8574 Pin Mapping to HD44780

| PCF8574 Pin | HD44780 Signal | Description              |
|-------------|----------------|--------------------------|
| P0          | D4             | Data bit 4               |
| P1          | D5             | Data bit 5               |
| P2          | D6             | Data bit 6               |
| P3          | D7             | Data bit 7               |
| P4          | RS             | Register Select          |
| P5          | RW             | Read/Write (GND)         |
| P6          | EN             | Enable                   |
| P7          | BL             | Backlight Control        |

## Communication Protocol

### I2C Layer

The driver communicates with the PCF8574 via Linux I2C user-space API:

```cpp
// Open I2C device
fd = open("/dev/i2c-1", O_RDWR);
ioctl(fd, I2C_SLAVE, 0x27);
write(fd, &data, 1);
```

### 4-bit Mode Data Transfer

In 4-bit mode, each byte is transmitted as two nibbles:

```
Byte:  [ D7 D6 D5 D4 | D3 D2 D1 D0 ]
        |   Upper     |   Lower      |
        v             v              v
       Nibble 1      Nibble 2       Nibble 3
       (bits 7-4)    (bits 3-0)      (next byte)
```

### Enable Pulse Generation

```
         +-----+     +-----+
 EN: ____|     |_____|     |____
        +-----+     +-----+
           |           |
      Data valid   Data latched
```

Timing requirements:
- Enable pulse width: min 450 ns
- Address setup time: min 60 ns
- Data setup time: min 195 ns
- Data hold time: min 10 ns

Implementation:

```cpp
void LCD::pulseEnable(uint8_t data) {
    expanderWrite(data | LCD_EN);
    usleep(1);              // 1 us > 450 ns
    expanderWrite(data & ~LCD_EN);
    usleep(1);
}
```

## Initialization Sequence

The HD44780 requires a specific initialization sequence after power-up:

```
State: Power-On Reset
  |
  |-- Wait > 15 ms (VCC rise time)
  |
  v
State: Reset Sequence (8-bit mode emulation)
  |
  |-- Send 0x03 (upper nibble only) -> Wait 4.1 ms
  |-- Send 0x03 (upper nibble only) -> Wait 100 us
  |-- Send 0x03 (upper nibble only) -> Wait 40 us
  |
  v
State: Set 4-bit Mode
  |
  |-- Send 0x02 (upper nibble only) -> Wait 40 us
  |
  v
State: Configuration
  |
  |-- Function Set (4-bit, 2-line, 5x8 dots)
  |-- Display Control (Display ON, Cursor OFF, Blink OFF)
  |-- Clear Display -> Wait 1.52 ms
  |-- Entry Mode Set (Increment, No shift)
  |
  v
State: Ready
```

Flow Diagram:

```
+----------------+
| Power-On Reset |
+-------+--------+
        |
        | Wait >15ms
        v
+-------+--------+
| Send 0x03      |<------------------+
+-------+--------+                   |
        |                            |
        | Wait >4.1ms                |
        v                            |
+-------+--------+                   |
| Send 0x03      |                   |
+-------+--------+                   |
        |                            |
        | Wait >100us                |
        v                            |
+-------+--------+                   |
| Send 0x03      |                   |
+-------+--------+                   |
        |                            |
        | Wait >40us                 |
        v                            |
+-------+--------+                   |
| Send 0x02      |                   |
| (4-bit mode)   |                   |
+-------+--------+                   |
        |                            |
        | Wait >40us                 |
        v                            |
+-------+--------+                   |
| Function Set   |                   |
| 0x28           |                   |
+-------+--------+                   |
        |                            |
        | Wait >40us                 |
        v                            |
+-------+--------+                   |
| Display Ctrl   |                   |
| 0x0C           |                   |
+-------+--------+                   |
        |                            |
        | Wait >40us                 |
        v                            |
+-------+--------+                   |
| Clear Display  |                   |
+-------+--------+                   |
        |                            |
        | Wait >1.52ms               |
        v                            |
+-------+--------+                   |
| Entry Mode     |                   |
| 0x06           |                   |
+-------+--------+                   |
        |                            |
        | Wait >40us                 |
        +----------------------------+
        |
        v
+-------+--------+
| Ready State    |
| (Idle)         |
+----------------+
```

## LCD Controller State Machine

The HD44780 controller operates as a state machine with the following states:

```
+----------------+
|   IDLE State   |<------------------+
| (Waiting for   |                   |
|  RS/ RW / Data)|                   |
+-------+--------+                   |
        |                            |
   RS=0 | RW=0 |                    RS=1 | RW=0
        |                            |
        v                            |
+-------+--------+                   |
| COMMAND State  |                   |
| (Execute       |                   |
|  Instruction)  |                   |
+-------+--------+                   |
        |                            |
        | Busy Flag Check            |
        v                            |
+-------+--------+                   |
| BUSY State     |                   |
| (BF=1, AC=...) |                   |
+-------+--------+                   |
        |                            |
        | BF=0                       |
        +----------------------------+
        |
        v
+-------+--------+
|   WRITE State  |
| (Write to      |
|  DDRAM/CGRAM)  |
+-------+--------+
        |
        | BF=0
        +---------------------------+
        |
        v
+----------------+
|   READ State   |
| (Read from     |
|  DDRAM/CGRAM)  |
+----------------+
```

### State Descriptions

1. **IDLE State**: The LCD waits for a valid instruction or data. RS and RW determine the next state.
2. **COMMAND State**: The LCD latches an instruction from the data lines and begins execution.
3. **BUSY State**: The LCD sets the Busy Flag (BF=1) and executes the current instruction. No new instructions are accepted until BF=0.
4. **WRITE State**: Data is written to DDRAM or CGRAM at the current address. AC is incremented or decremented based on entry mode.
5. **READ State**: Data is read from DDRAM or CGRAM at the current address. AC is incremented or decremented based on entry mode.

### Address Counter (AC)

The Address Counter points to the current DDRAM or CGRAM address:
- After power-on: AC = 0x00
- After Clear Display: AC = 0x00
- After Return Home: AC = 0x00
- After data write: AC = AC +/- 1 (based on entry mode)
- After Set DDRAM/CGRAM Address: AC = specified address

## DDRAM Memory Map for 20x4 Display

```
Row 0: 0x00 - 0x13 (20 characters)
Row 1: 0x40 - 0x53 (20 characters)
Row 2: 0x14 - 0x27 (20 characters)
Row 3: 0x54 - 0x67 (20 characters)
```

```
DDRAM Address Map:
  0x00  1  2  3 ... 19
  0x40  1  2  3 ... 19
  0x14  1  2  3 ... 19
  0x54  1  2  3 ... 19
```

## CGROM Character Generator

The HD44780 includes a built-in CGROM with 208 characters:

- 8-bit ASCII characters (0x20 - 0x7F)
- Japanese characters (0xA0 - 0xDF)
- Custom characters can be defined in CGRAM (8 characters, 8 bytes each)

### CGRAM Address Calculation

```
CGRAM Address = 0x40 | (slot << 3) | byte_index
```

Where:
- `slot`: Character slot (0-7)
- `byte_index`: Row within character (0-7)

Example for slot 0, row 3:
```
CGRAM Address = 0x40 | (0 << 3) | 3 = 0x43
```

## Timing Analysis

| Operation            | Minimum Delay | Driver Delay | Notes                          |
|---------------------|---------------|--------------|--------------------------------|
| Enable Pulse Width   | 450 ns        | 1 us         | usleep(1)                      |
| Address Setup        | 60 ns         | N/A          | Handled by hardware            |
| Data Setup           | 195 ns        | N/A          | Handled by hardware            |
| Data Hold            | 10 ns         | N/A          | Handled by hardware            |
| Command Execution    | 37 us         | 37 us        | usleep(37)                     |
| Clear Display        | 1.52 ms       | 1.52 ms      | usleep(1520)                   |
| Return Home          | 1.52 ms       | 1.52 ms      | usleep(1520)                   |
| Power-On Init        | 15 ms         | 50 ms        | Conservative margin            |

## Error Handling Strategy

The driver uses C++ exceptions for error handling:

- `std::runtime_error`: I2C communication failures, file descriptor errors
- `std::invalid_argument`: Invalid cursor positions, invalid character slots

All I2C operations validate return values and throw descriptive exceptions including `errno` descriptions.

## Thread Safety

This driver is NOT thread-safe. Concurrent access from multiple threads requires external synchronization (e.g., `std::mutex`).

Example usage with mutex:

```cpp
#include <mutex>

std::mutex lcd_mutex;

void safeWrite(LCD& lcd, const std::string& text) {
    std::lock_guard<std::mutex> lock(lcd_mutex);
    lcd.writeString(text);
}
```
