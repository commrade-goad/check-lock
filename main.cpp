#include <cstdio>
#include <fcntl.h>
#include <linux/input.h>
#include <iostream>
#include <string>
#include <dirent.h>
#include <unistd.h>
#include <sys/ioctl.h>

enum LedMode {
    CAPSLOCK,
    NUMLOCK
};

std::string getKeyboardDevice() {
    std::string path = "/dev/input/by-path/";
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        std::cerr << "ERR: Failed to open directory: " << path << std::endl;
        return "";
    }

    struct dirent* entry;
    std::string keyboard_device;

    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_LNK && std::string(entry->d_name).find("-event-kbd") != std::string::npos) {
            keyboard_device = path + entry->d_name;
            break;
        }
    }

    closedir(dir);
    return keyboard_device;
}

bool openFile(std::string device, unsigned short mode, int &fd, unsigned long &leds) {
    if (device.empty()) {
        std::cerr << "ERR: Keyboard device not found." << std::endl;
        return 1;
    }

    fd = open(device.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "ERR: Error opening device" << std::endl;
        return 1;
    }

    if (ioctl(fd, EVIOCGLED(sizeof(leds)), &leds) == -1) {
        std::cerr << "ERR: Error reading LED state" << std::endl;
        close(fd);
        return 1;
    }

    return 0;
}

bool parseArgs(int argc, char **argv, LedMode &mode) {
    if (argc < 2) {
        std::cerr << "ERR: Not enought argument!" << std::endl;
        return 1;
    }

    for (int i = 0; i < argc; i++) {
        std::string mode = argv[i];
        if (mode == "-c" || mode == "--capslock") {
            return LedMode::CAPSLOCK;
        }
        if (mode == "-n" || mode == "--numlock") {
            return LedMode::NUMLOCK;
        }
    }

    return true;
}

int main(int argc, char **argv) {
    int fd;
    unsigned long leds;
    LedMode mode;

    bool success = parseArgs(argc, argv, mode);
    if (!success) {
        return -1;
    }

    std::string device = getKeyboardDevice();
    success = openFile(device, O_RDONLY, fd, leds);
    if (!success) {
        close(fd);
        return -1;
    }

    switch (mode) {
        case LedMode::CAPSLOCK:
            if (leds & (1 << LED_CAPSL)) {
                std::cout << "true" << std::endl;
            } else {
                std::cout << "false" << std::endl;
            }
            break;
        case LedMode::NUMLOCK:
            if (leds & (1 << LED_NUML)) {
                std::cout << "true" << std::endl;
            } else {
                std::cout << "false" << std::endl;
            }
            break;
        default:
            break;
    }

    close(fd);
    return 0;
}
