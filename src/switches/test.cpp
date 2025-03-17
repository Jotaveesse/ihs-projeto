#include "buttons.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

int main() {
    int fileDescriptor = 0;
    unsigned int command = 0;
    unsigned int buttonCount = 4;

    Buttons buttons(fileDescriptor, command, buttonCount);

    int choice;
    unsigned int buttonIndex, duration;

    while (true) {
        std::cout << "\nMenu:\n";
        std::cout << "1. Read Buttons\n";
        std::cout << "2. Check Button Pressed\n";
        std::cout << "3. Check Button Clicked\n";
        std::cout << "4. Check Button Released\n";
        std::cout << "5. Check Button Pressed Long\n";
        std::cout << "6. Get Button States as Number\n";
        std::cout << "7. Get Button States as Array\n";
        std::cout << "8. Get Button Count\n";
        std::cout << "9. Print Button States\n";
        std::cout << "10. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                buttons.update();
                buttons.printStates();
                break;
            case 2:
                std::cout << "Enter button index: ";
                std::cin >> buttonIndex;
                std::cout << "Button " << buttonIndex << " pressed: " << buttons.isButtonPressed(buttonIndex) << std::endl;
                buttons.printStates();
                break;
            case 3:
                std::cout << "Enter button index: ";
                std::cin >> buttonIndex;
                std::cout << "Button " << buttonIndex << " clicked: " << buttons.isButtonClicked(buttonIndex) << std::endl;
                buttons.printStates();
                break;
            case 4:
                std::cout << "Enter button index: ";
                std::cin >> buttonIndex;
                std::cout << "Button " << buttonIndex << " released: " << buttons.isButtonReleased(buttonIndex) << std::endl;
                buttons.printStates();
                break;
            case 5:
                std::cout << "Enter button index: ";
                std::cin >> buttonIndex;
                std::cout << "Enter duration (milliseconds): ";
                std::cin >> duration;
                std::cout << "Button " << buttonIndex << " pressed long: " << buttons.isButtonPressedLong(buttonIndex, duration) << std::endl;
                buttons.printStates();
                break;
            case 6:
                std::cout << "Button states as number: " << buttons.getStatesAsNumber() << std::endl;
                buttons.printStates();
                break;
            case 7: {
                std::vector<unsigned int> states = buttons.getStatesArray();
                std::cout << "Button states as array: ";
                for (unsigned int state : states) {
                    std::cout << state << " ";
                }
                std::cout << std::endl;
                buttons.printStates();
                break;
            }
            case 8:
                std::cout << "Button count: " << buttons.getCount() << std::endl;
                buttons.printStates();
                break;
            case 9:
                buttons.printStates();
                break;
            case 10:
                return 0;
            default:
                std::cout << "Invalid choice!\n";
        }
    }

    return 0;
}