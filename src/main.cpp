#include "lcd.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <cmath>

volatile sig_atomic_t running = 1;

void signalHandler(int) {
    running = 0;
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        LCD lcd("/dev/i2c-1", 0x27);
        lcd.init();
        lcd.backlightOn();
        lcd.clear();
        lcd.home();
        lcd.displayOn();
        lcd.cursorOff();
        lcd.blinkOff();

        lcd.writeString("Hola Mundo");
        lcd.setCursor(0, 1);
        lcd.writeString("Contador: ");

        int counter = 0;
        while (running) {
            lcd.setCursor(10, 1);
            lcd.writeString("    ");
            lcd.setCursor(10, 1);
            lcd.writeString(std::to_string(counter));
            counter++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        lcd.clear();
        lcd.home();
        lcd.writeString("Adios!");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        lcd.backlightOff();
        lcd.displayOff();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
