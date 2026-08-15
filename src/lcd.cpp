#include "lcd.hpp"
#include <cerrno>

LCD::LCD(const std::string& i2c_device, uint8_t address)
    : fd_(-1), address_(address), backlight_mask_(LCD_BACKLIGHT),
      display_control_(0), display_function_(0), display_mode_(0),
      num_lines_(4) {
    fd_ = open(i2c_device.c_str(), O_RDWR);
    if (fd_ < 0) {
        throw std::runtime_error("Failed to open I2C device: " + i2c_device + " (" + std::strerror(errno) + ")");
    }

    if (ioctl(fd_, I2C_SLAVE, address_) < 0) {
        close(fd_);
        fd_ = -1;
        throw std::runtime_error("Failed to acquire I2C bus access and/or talk to slave: 0x" +
                                 std::to_string(address_) + " (" + std::strerror(errno) + ")");
    }

    row_offsets_[0] = 0x00;
    row_offsets_[1] = 0x40;
    row_offsets_[2] = 0x14;
    row_offsets_[3] = 0x54;
}

LCD::~LCD() {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

void LCD::i2cWrite(uint8_t data) {
    if (write(fd_, &data, 1) != 1) {
        throw std::runtime_error("I2C write failed (" + std::string(std::strerror(errno)) + ")");
    }
}

void LCD::expanderWrite(uint8_t data) {
    i2cWrite(data | backlight_mask_);
}

void LCD::pulseEnable(uint8_t data) {
    expanderWrite(data | LCD_EN);
    usleep(DELAY_ENABLE);
    expanderWrite(data & ~LCD_EN);
    usleep(DELAY_ENABLE);
}

void LCD::write4bits(uint8_t data) {
    expanderWrite(data);
    pulseEnable(data);
}

void LCD::send(uint8_t value, uint8_t mode) {
    write4bits((value & 0xF0) | mode);
    write4bits((value << 4) | mode);
}

void LCD::command(uint8_t cmd) {
    send(cmd, 0);
}

void LCD::write(uint8_t value) {
    send(value, LCD_RS);
}

void LCD::init() {
    i2cWrite(0);
    usleep(50000);

    write4bits(0x03);
    usleep(4500);
    write4bits(0x03);
    usleep(450);
    write4bits(0x03);
    usleep(450);
    write4bits(0x02);
    usleep(450);

    display_function_ = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
    command(LCD_FUNCTIONSET | display_function_);
    usleep(4500);

    display_control_ = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    command(LCD_DISPLAYCONTROL | display_control_);
    usleep(450);

    clear();
    usleep(DELAY_CLEAR);

    display_mode_ = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    command(LCD_ENTRYMODESET | display_mode_);
    usleep(450);
}

void LCD::clear() {
    command(LCD_CLEARDISPLAY);
    usleep(DELAY_CLEAR);
}

void LCD::home() {
    command(LCD_RETURNHOME);
    usleep(DELAY_HOME);
}

void LCD::setCursor(uint8_t col, uint8_t row) {
    if (row >= num_lines_) {
        row = num_lines_ - 1;
    }
    if (col >= 20) {
        col = 19;
    }

    command(LCD_SETDDRAMADDR | (col + row_offsets_[row]));
    usleep(DELAY_CMD);
}

void LCD::writeChar(char c) {
    write(static_cast<uint8_t>(c));
    usleep(DELAY_CMD);
}

void LCD::writeString(const std::string& str) {
    for (char c : str) {
        writeChar(c);
    }
}

void LCD::createChar(uint8_t slot, const uint8_t* charmap) {
    slot &= 0x07;
    command(LCD_SETCGRAMADDR | (slot << 3));
    usleep(DELAY_CMD);
    for (uint8_t i = 0; i < 8; i++) {
        write(charmap[i]);
    }
}

void LCD::displayOn() {
    display_control_ |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | display_control_);
    usleep(DELAY_CMD);
}

void LCD::displayOff() {
    display_control_ &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | display_control_);
    usleep(DELAY_CMD);
}

void LCD::cursorOn() {
    display_control_ |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | display_control_);
    usleep(DELAY_CMD);
}

void LCD::cursorOff() {
    display_control_ &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | display_control_);
    usleep(DELAY_CMD);
}

void LCD::blinkOn() {
    display_control_ |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | display_control_);
    usleep(DELAY_CMD);
}

void LCD::blinkOff() {
    display_control_ &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | display_control_);
    usleep(DELAY_CMD);
}

void LCD::backlightOn() {
    backlight_mask_ |= LCD_BACKLIGHT;
    expanderWrite(0);
}

void LCD::backlightOff() {
    backlight_mask_ &= ~LCD_BACKLIGHT;
    expanderWrite(0);
}
