#ifndef LCD_HPP
#define LCD_HPP

#include <cstdint>
#include <string>
#include <stdexcept>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

class LCD {
public:
    explicit LCD(const std::string& i2c_device, uint8_t address);
    ~LCD();

    void init();
    void clear();
    void home();
    void setCursor(uint8_t col, uint8_t row);
    void writeChar(char c);
    void writeString(const std::string& str);
    void createChar(uint8_t slot, const uint8_t* charmap);

    void displayOn();
    void displayOff();
    void cursorOn();
    void cursorOff();
    void blinkOn();
    void blinkOff();
    void backlightOn();
    void backlightOff();

    int getFd() const { return fd_; }

private:
    void i2cWrite(uint8_t data);
    void expanderWrite(uint8_t data);
    void pulseEnable(uint8_t data);
    void write4bits(uint8_t data);
    void send(uint8_t value, uint8_t mode);
    void command(uint8_t cmd);
    void write(uint8_t value);

    int fd_;
    uint8_t address_;
    uint8_t backlight_mask_;
    uint8_t display_control_;
    uint8_t display_function_;
    uint8_t display_mode_;
    uint8_t num_lines_;
    uint8_t row_offsets_[4];

    static constexpr uint8_t LCD_4BITMODE = 0x00;
    static constexpr uint8_t LCD_1LINE = 0x00;
    static constexpr uint8_t LCD_2LINE = 0x08;
    static constexpr uint8_t LCD_5x8DOTS = 0x00;
    static constexpr uint8_t LCD_5x10DOTS = 0x04;

    static constexpr uint8_t LCD_CLEARDISPLAY = 0x01;
    static constexpr uint8_t LCD_RETURNHOME = 0x02;
    static constexpr uint8_t LCD_ENTRYMODESET = 0x04;
    static constexpr uint8_t LCD_DISPLAYCONTROL = 0x08;
    static constexpr uint8_t LCD_CURSORSHIFT = 0x10;
    static constexpr uint8_t LCD_FUNCTIONSET = 0x20;
    static constexpr uint8_t LCD_SETCGRAMADDR = 0x40;
    static constexpr uint8_t LCD_SETDDRAMADDR = 0x80;

    static constexpr uint8_t LCD_ENTRYRIGHT = 0x00;
    static constexpr uint8_t LCD_ENTRYLEFT = 0x02;
    static constexpr uint8_t LCD_ENTRYSHIFTINCREMENT = 0x01;
    static constexpr uint8_t LCD_ENTRYSHIFTDECREMENT = 0x00;

    static constexpr uint8_t LCD_DISPLAYON = 0x04;
    static constexpr uint8_t LCD_DISPLAYOFF = 0x00;
    static constexpr uint8_t LCD_CURSORON = 0x02;
    static constexpr uint8_t LCD_CURSOROFF = 0x00;
    static constexpr uint8_t LCD_BLINKON = 0x01;
    static constexpr uint8_t LCD_BLINKOFF = 0x00;

    static constexpr uint8_t LCD_8BITMODE = 0x10;

    static constexpr uint8_t LCD_RS = 0x10;
    static constexpr uint8_t LCD_RW = 0x20;
    static constexpr uint8_t LCD_EN = 0x40;
    static constexpr uint8_t LCD_BACKLIGHT = 0x80;

    static constexpr uint32_t DELAY_ENABLE = 50;
    static constexpr uint32_t DELAY_CMD = 37;
    static constexpr uint32_t DELAY_CLEAR = 1520;
    static constexpr uint32_t DELAY_HOME = 1520;
};

#endif
