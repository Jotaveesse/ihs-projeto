#include "lcd/lcd.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "ioctl_cmds.h"
#include <vector>
#include <string>

int main() {
    nt fileDescriptor = -1;

    fileDescriptor = open("/dev/mydev", O_RDWR);
    if (fileDescriptor < 0) {
        std::cerr << "Failed to open device: " << strerror(errno) << std::endl;
        return 1;
    }
    unsigned int command = WR_LCD_DISPLAY;

    LCD lcd(fileDescriptor, command);

    int choice;
    int col, row;
    std::string text;
    unsigned int cmd, flags, address, data;

    lcd.init(); // Initialize the LCD

    while (true) {
        std::cout << "\nLCD Menu:\n";
        std::cout << "1. Turn On\n";
        std::cout << "2. Turn Off\n";
        std::cout << "3. Clear\n";
        std::cout << "4. Home\n";
        std::cout << "5. Set Cursor\n";
        std::cout << "6. Write Text\n";
        std::cout << "7. Write Character\n";
        std::cout << "8. Send Command\n";
        std::cout << "9. Set Entry Mode\n";
        std::cout << "10. Set Display Control\n";
        std::cout << "11. Shift\n";
        std::cout << "12. Set CGRAM Address\n";
        std::cout << "13. Set DDRAM Address\n";
        std::cout << "14. Read Busy Flag and Address\n";
        std::cout << "15. Write Data\n";
        std::cout << "16. Read Data\n";
        std::cout << "17. Is Busy?\n";
        std::cout << "18. Update\n";
        std::cout << "19. Print States\n";
        std::cout << "20. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                lcd.turnOn();
                std::cout << "LCD turned ON.\n";
                break;
            case 2:
                lcd.turnOff();
                std::cout << "LCD turned OFF.\n";
                break;
            case 3:
                lcd.clear();
                std::cout << "LCD cleared.\n";
                break;
            case 4:
                lcd.home();
                std::cout << "Cursor returned home.\n";
                break;
            case 5:
                std::cout << "Enter column: ";
                std::cin >> col;
                std::cout << "Enter row: ";
                std::cin >> row;
                lcd.setCursor(col, row);
                std::cout << "Cursor set to (" << col << ", " << row << ").\n";
                break;
            case 6:
                std::cout << "Enter text to write: ";
                std::cin.ignore(); // Clear newline from previous input
                std::getline(std::cin, text);
                lcd.write(text);
                std::cout << "Text written.\n";
                break;
            case 7:
                std::cout << "Enter character to write: ";
                std::cin.ignore(); // Clear newline
                char c;
                std::cin.get(c);
                lcd.write(c);
                std::cout << "Character written.\n";
                break;
            case 8:
                std::cout << "Enter command (hex): ";
                std::cin >> std::hex >> cmd;
                lcd.command(cmd);
                std::cout << "Command sent.\n";
                break;
            case 9:
                std::cout << "Enter entry mode flags (hex): ";
                std::cin >> std::hex >> flags;
                lcd.setEntryMode(flags);
                std::cout << "Entry mode set.\n";
                break;
            case 10:
                std::cout << "Enter display control flags (hex): ";
                std::cin >> std::hex >> flags;
                lcd.setDisplayControl(flags);
                std::cout << "Display control set.\n";
                break;
            case 11:
                std::cout << "Enter shift flags (hex): ";
                std::cin >> std::hex >> flags;
                lcd.shift(flags);
                std::cout << "Shift command sent.\n";
                break;
            case 12:
                std::cout << "Enter CGRAM address (hex): ";
                std::cin >> std::hex >> address;
                lcd.setCGRAMAddress(address);
                std::cout << "CGRAM address set.\n";
                break;
            case 13:
                std::cout << "Enter DDRAM address (hex): ";
                std::cin >> std::hex >> address;
                lcd.setDDRAMAddress(address);
                std::cout << "DDRAM address set.\n";
                break;
            case 14:
                std::cout << "Busy flag and address: 0x" << std::hex << static_cast<int>(lcd.readBusyFlagAndAddress()) << std::dec << std::endl;
                break;
            case 15:
                std::cout << "Enter data to write (hex): ";
                std::cin >> std::hex >> data;
                lcd.writeData(data);
                std::cout << "Data written.\n";
                break;
            case 16:
                std::cout << "Data read: 0x" << std::hex << static_cast<int>(lcd.readData()) << std::dec << std::endl;
                break;
            case 17:
                std::cout << "LCD is busy: " << (lcd.isBusy() ? "Yes" : "No") << std::endl;
                break;
            case 18:
                lcd.update();
                std::cout << "LCD updated.\n";
                break;
            case 19:
                lcd.printStates();
                break;
            case 20:
                return 0;
            default:
                std::cout << "Invalid choice!\n";
        }
    }

    return 0;
}