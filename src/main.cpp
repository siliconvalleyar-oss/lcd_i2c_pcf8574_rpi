#include "lcd.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <poll.h>
#include <termios.h>
#include <unistd.h>
#include <iomanip>
#include <sstream>
#include <ctime>

volatile sig_atomic_t running = 1;

void signalHandler(int) {
    running = 0;
}

void setTerminalRaw(bool enable) {
    static struct termios oldt = {};
    static bool initialized = false;

    if (enable) {
        if (!initialized) {
            tcgetattr(STDIN_FILENO, &oldt);
            initialized = true;
        }
        struct termios newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else if (initialized) {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

bool waitForKey(char& c, int timeout_ms) {
    struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
    int ret = poll(&pfd, 1, timeout_ms);
    if (ret > 0 && (pfd.revents & POLLIN)) {
        if (read(STDIN_FILENO, &c, 1) == 1) {
            return true;
        }
    }
    return false;
}

std::string currentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%H:%M:%S");
    return ss.str();
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        setTerminalRaw(true);

        LCD lcd("/dev/i2c-1", 0x27);
        lcd.init();
        lcd.backlightOn();
        lcd.displayOn();
        lcd.cursorOff();
        lcd.blinkOff();

        char key = 0;
        while (running) {
            lcd.clear();
            lcd.home();
            lcd.writeString("=== LCD Menu ===");
            lcd.setCursor(0, 1);
            lcd.writeString("1. Contador");
            lcd.setCursor(0, 2);
            lcd.writeString("2. Hora");
            lcd.setCursor(0, 3);
            lcd.writeString("q. Salir");

            while (running) {
                if (waitForKey(key, 200)) {
                    if (key == 'q' || key == 'Q') {
                        running = 0;
                        break;
                    }
                    if (key == '1') {
                        lcd.clear();
                        lcd.home();
                        lcd.writeString("Contador:");
                        int counter = 0;
                        while (running) {
                            lcd.setCursor(0, 1);
                            lcd.writeString("                ");
                            lcd.setCursor(0, 1);
                            lcd.writeString(std::to_string(counter));
                            counter++;
                            if (waitForKey(key, 1000)) {
                                if (key == 'q' || key == 'Q' || key == 27) {
                                    running = 0;
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    if (key == '2') {
                        lcd.clear();
                        lcd.home();
                        lcd.writeString("Hora actual:");
                        while (running) {
                            lcd.setCursor(0, 1);
                            lcd.writeString("                ");
                            lcd.setCursor(0, 1);
                            lcd.writeString(currentTime());
                            if (waitForKey(key, 1000)) {
                                if (key == 'q' || key == 'Q' || key == 27) {
                                    running = 0;
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }

        lcd.clear();
        lcd.home();
        lcd.writeString("Adios!");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        lcd.backlightOff();
        lcd.displayOff();
        setTerminalRaw(false);

    } catch (const std::exception& e) {
        setTerminalRaw(false);
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
