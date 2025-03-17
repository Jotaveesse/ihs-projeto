#include "seven_segment_displays.h"
#include <iostream>
#include <vector>

int main() {
    int fileDescriptor = 0;
    unsigned int commandLeft = 0;
    unsigned int commandRight = 1;
    unsigned int segCount = 8;
    SevenSegmentDisplays displays(fileDescriptor, commandLeft, commandRight, segCount);

    int choice;
    unsigned int displayIndex, number;
    std::vector<unsigned int> numbers;

    while (true) {
        std::cout << "\nMenu:\n";
        std::cout << "1. Set Display from Number\n";
        std::cout << "2. Set All Displays from Number\n";
        std::cout << "3. Set All Displays from Number Array\n";
        std::cout << "4. Update Displays\n";
        std::cout << "5. Print Displays\n";
        std::cout << "6. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "Enter display index (0-7): ";
                std::cin >> displayIndex;
                std::cout << "Enter number (0-15): ";
                std::cin >> number;
                displays.setDisplayFromNumber(displayIndex, number);
                break;
            case 2:
                std::cout << "Enter number: ";
                std::cin >> number;
                displays.setAllDisplaysFromNumber(number);
                break;
            case 3:
                std::cout << "Enter numbers (separated by spaces, max 8): ";
                numbers.clear();
                for (int i = 0; i < segCount; ++i) {
                    unsigned int digit;
                    std::cin >> digit;
                    numbers.push_back(digit);
                }
                displays.setAllDisplaysFromNumberArray(numbers);
                break;
            case 4:
                displays.update();
                break;
            case 5:
                displays.printDisplays();
                break;
            case 6:
                return 0;
            default:
                std::cout << "Invalid choice!\n";
        }
        displays.printStates();
    }
    return 0;
}