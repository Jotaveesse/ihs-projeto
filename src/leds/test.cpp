#include "leds.h"
#include <iostream>
#include <vector>

int main() {
    int fileDescriptor = 0;
    unsigned int command = 0;
    unsigned int ledCount = 18;
    Leds red_leds(fileDescriptor, command, ledCount);

    int choice;
    unsigned int ledIndex, number;

    while (true) {
        std::cout << "\nMenu:\n";
        std::cout << "1. Turn LED On\n";
        std::cout << "2. Turn LED Off\n";
        std::cout << "3. Set LEDs from Number\n";
        std::cout << "4. Get LED States as Number\n";
        std::cout << "5. Set LEDs from Array\n";
        std::cout << "6. Turn All LEDs On\n";
        std::cout << "7. Turn All LEDs Off\n";
        std::cout << "8. Print LED States\n";
        std::cout << "9. Update LEDs\n";
        std::cout << "10. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "Enter LED index to turn on: ";
                std::cin >> ledIndex;
                red_leds.setState(ledIndex, true);
                red_leds.printStates();
                break;
            case 2:
                std::cout << "Enter LED index to turn off: ";
                std::cin >> ledIndex;
                red_leds.setState(ledIndex, false);
                red_leds.printStates();
                break;
            case 3:
                std::cout << "Enter number to set LEDs: ";
                std::cin >> number;
                red_leds.setStatesFromNumber(number);
                red_leds.printStates();
                break;
            case 4:
                std::cout << "LED states as number: " << red_leds.getStatesAsNumber() << std::endl;
                break;
            case 5: {
                std::vector<bool> boolArray(ledCount);
                std::cout << "Enter " << ledCount << " boolean values (0 or 1) separated by spaces: ";
                for (int i = ledCount - 1; i >= 0; --i) {
                    int val;
                    std::cin >> val;
                    boolArray[i] = (val != 0);
                }
                red_leds.setStatesFromArray(boolArray);
                red_leds.printStates();
                break;
            }
            case 6:
                red_leds.setAllStates(true);
                red_leds.printStates();
                break;
            case 7:
                red_leds.setAllStates(false);
                red_leds.printStates();
                break;
            case 8:
                red_leds.printStates();
                break;
            case 9:
                red_leds.update();
                red_leds.printStates();
                break;
            case 10:
                return 0;
            default:
                std::cout << "Invalid choice!\n";
        }
    }

    return 0;
}