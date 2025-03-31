#include "peripherals/buttons/buttons.h"
#include "peripherals/switches/switches.h"
#include "peripherals/leds/leds.h"
#include "peripherals/seven_segment_displays/seven_segment_displays.h"
#include "peripherals/lcd/lcd.h"

#include "constants.h"
#include "utils.h"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "ioctl_cmds.h"
#include <chrono>
#include <random>
#include <algorithm>

#include <thread>
#include <omp.h>
#include <mutex>

int modulesDeactivated = 0;
const int buttonPressDelay = 1100;

std::mutex timerMutex;
std::mutex deviceMutex;

void id_module(SevenSegmentDisplays *sevenSegment)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> idDist(0, 15);

    int num1 = idDist(gen);
    int num2 = idDist(gen);

    sevenSegment->setDisplayFromNumber(5, num2);
    sevenSegment->setDisplayFromNumber(4, num1);
}

void timer_module(SevenSegmentDisplays *sevenSegment, int *timer)
{
    int initialTimer = *timer;
    unsigned int startTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    unsigned int currTime = startTime;

    while (modulesDeactivated < 4 && *timer > 0)
    {
        currTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        subtractTimer(timer, (currTime - startTime), timerMutex);
        startTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        int dig0 = (*timer / 1000000) % 10;
        int dig1 = ((*timer / 1000000) % 60) / 10;
        int dig2 = ((*timer / 1000000) / 60) % 10;
        int dig3 = ((*timer / 1000000) / 600) % 10;

        sevenSegment->setDisplayFromNumber(0, dig0);
        sevenSegment->setDisplayFromNumber(1, dig1);
        sevenSegment->setDisplayFromNumber(2, dig2);
        sevenSegment->setDisplayFromNumber(3, dig3);

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            sevenSegment->update();
        }
    }
}

void buttons_module(Buttons *buttons, int *timer)
{
    while (modulesDeactivated < 4 && *timer > 0)
    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        buttons->update();
    }
}

void switches_module(Switches *switches, int *timer)
{
    while (modulesDeactivated < 4 && *timer > 0)
    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        switches->update();
    }
}

bool red_leds_module(Buttons *buttons, Switches *switches, Leds *redLeds, SevenSegmentDisplays *sevenSegment, int *timer)
{
    unsigned int switchesStates = 0;
    unsigned int buttonStates = 0;

    bool heldButton = false;
    bool deactivated = false;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 2);

    std::vector<int> ledModes(redLeds->getCount());

    for (unsigned int i = 0; i < redLeds->getCount(); ++i)
    {
        ledModes[i] = dist(gen);
    }

    std::string id = getIdString(sevenSegment);

    std::cout << "red leds: " << intToBinaryString(getCombinationRedLeds(ledModes, id)) << std::endl;

    while (!deactivated && *timer > 0)
    {
        buttonStates = buttons->getStatesAsNumber();
        switchesStates = switches->getStatesAsNumber();

        id = getIdString(sevenSegment);

        if (buttons->isButtonPressedLong(3, buttonPressDelay))
        {
            heldButton = true;
        }

        for (unsigned int i = 0; i < redLeds->getCount(); ++i)
        {
            switch (ledModes[i])
            {
            case 0: // Off
                redLeds->setState(i, false);
                break;
            case 1: // On
                redLeds->setState(i, true);
                break;
            case 2: // Blink
                redLeds->blink(i, 1000);
                break;
            }
        }

        // confirmação da escolha
        if (heldButton && !buttons->isButtonPressed(3))
        {
            heldButton = false;

            unsigned int correctCombination = getCombinationRedLeds(ledModes, id);

            if (correctCombination == switchesStates)
            {
                deactivated = true;
            }
            else
            {
                subtractTimer(timer, 15000000, timerMutex);
            }
        }
        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            redLeds->update();
        }
    }

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        redLeds->setAllStates(*timer <= 0);
        redLeds->update();
    }

    return deactivated;
}

bool green_leds_module(Buttons *buttons, Switches *switches, Leds *greenLeds, int *timer)
{
    unsigned int buttonStates = 0;
    unsigned int switchesStates = 0;
    bool heldButton = false;
    bool deactivated = false;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 4);

    std::vector<int> blinkPeriods(greenLeds->getCount());

    for (unsigned int i = 0; i < greenLeds->getCount(); ++i)
    {
        int period = dist(gen);
        blinkPeriods[i] = period * 1000;
    }

    int combination = getCombinationGreenLeds(blinkPeriods);

    std::cout << "green leds: " << intToBinaryString(combination) << std::endl;

    while (!deactivated && *timer > 0)
    {
        buttonStates = buttons->getStatesAsNumber();
        switchesStates = switches->getStatesAsNumber();

        if (buttons->isButtonPressedLong(2, buttonPressDelay))
        {
            heldButton = true;
        }

        for (unsigned int i = 0; i < greenLeds->getCount(); ++i)
        {
            greenLeds->blink(i, blinkPeriods[i]);
        }

        // confirmação da escolha
        if (heldButton && !buttons->isButtonPressed(2))
        {
            heldButton = false;
            bool correctCombination = switches->getStatesAsNumber() == combination;

            if (correctCombination)
            {
                deactivated = true;
            }
            else
            {
                subtractTimer(timer, 15000000, timerMutex);
            }
        }
        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            greenLeds->update();
        }
    }

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        greenLeds->setAllStates(*timer <= 0);
        greenLeds->update();
    }

    return deactivated;
}

bool seven_segment_module(Buttons *buttons, Switches *switches, SevenSegmentDisplays *sevenSegment, int *timer)
{
    unsigned int switchesStates = 0;
    unsigned int buttonStates = 0;

    bool deactivated = false;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 4);

    int stage = 0;
    std::vector<int> displayedNumbers(4);
    std::vector<int> pressedPositions(4);

    for (unsigned int i = 0; i < 4; ++i)
    {
        int num = dist(gen);
        displayedNumbers[i] = num;
    }

    std::array<int, 4> correct = getCorrectCombinationMemory(displayedNumbers);

    std::cout << "memory:";
    for (int i = 0; i < correct.size(); i++)
    {
        std::cout << " " << correct[i];
    }
    std::cout << std::endl;

    bool buttonReleased = false;
    int buttonPressed = -1;
    int chosenButton = -1;

    while (!deactivated && *timer > 0)
    {
        sevenSegment->setDisplayFromNumber(6, stage + 1);
        sevenSegment->setDisplayFromNumber(7, displayedNumbers[stage]);

        buttonReleased = false;
        if (buttons->isButtonPressed(0))
        {
            buttonPressed = 0;
        }
        if (buttons->isButtonPressed(1))
        {
            buttonPressed = 1;
        }
        if (buttons->isButtonPressed(2))
        {
            buttonPressed = 2;
        }
        if (buttons->isButtonPressed(3))
        {
            buttonPressed = 3;
        }
        if (buttons->isButtonPressedLong(0, buttonPressDelay) || buttons->isButtonPressedLong(1, buttonPressDelay) || buttons->isButtonPressedLong(2, buttonPressDelay) || buttons->isButtonPressedLong(3, buttonPressDelay))
        {
            buttonPressed = -1;
        }

        if (buttonPressed != -1 && !buttons->isButtonPressed(buttonPressed) && !buttons->isButtonPressedLong(buttonPressed, buttonPressDelay))
        {
            chosenButton = 4 - buttonPressed;
            buttonReleased = true;
            buttonPressed = -1;
        }

        if (buttonReleased)
        {
            bool correctButton = false;
            switch (stage + 1)
            {
            case 1:
                switch (displayedNumbers[stage])
                {
                case 1:
                    correctButton = chosenButton == 2;
                    pressedPositions[stage] = 2;
                    break;
                case 2:
                    correctButton = chosenButton == 2;
                    pressedPositions[stage] = 2;
                    break;
                case 3:
                    correctButton = chosenButton == 1;
                    pressedPositions[stage] = 1;
                    break;
                case 4:
                    correctButton = chosenButton == 4;
                    pressedPositions[stage] = 4;
                    break;
                }
                break;
            case 2:
                switch (displayedNumbers[stage])
                {
                case 1:
                    correctButton = chosenButton == displayedNumbers[1 - 1];
                    pressedPositions[stage] = displayedNumbers[1 - 1];
                    break;
                case 2:
                    correctButton = chosenButton == pressedPositions[1 - 1];
                    pressedPositions[stage] = pressedPositions[1 - 1];
                    break;
                case 3:
                    correctButton = chosenButton == 1;
                    pressedPositions[stage] = 1;
                    break;
                case 4:
                    correctButton = chosenButton == pressedPositions[1 - 1];
                    pressedPositions[stage] = pressedPositions[1 - 1];
                    break;
                }
                break;
            case 3:
                switch (displayedNumbers[stage])
                {
                case 1:
                    correctButton = chosenButton == pressedPositions[2 - 1];
                    pressedPositions[stage] = pressedPositions[2 - 1];
                    break;
                case 2:
                    correctButton = chosenButton == pressedPositions[1 - 1];
                    pressedPositions[stage] = pressedPositions[1 - 1];
                    break;
                case 3:
                    correctButton = chosenButton == 3;
                    pressedPositions[stage] = 3;
                    break;
                case 4:
                    correctButton = chosenButton == displayedNumbers[2 - 1];
                    pressedPositions[stage] = displayedNumbers[2 - 1];
                    break;
                }
                break;
            case 4:
                switch (displayedNumbers[stage])
                {
                case 1:
                    correctButton = chosenButton == pressedPositions[2 - 1];
                    pressedPositions[stage] = pressedPositions[2 - 1];
                    break;
                case 2:
                    correctButton = chosenButton == pressedPositions[1 - 1];
                    pressedPositions[stage] = pressedPositions[1 - 1];
                    break;
                case 3:
                    correctButton = chosenButton == pressedPositions[2 - 1];
                    pressedPositions[stage] = pressedPositions[2 - 1];
                    break;
                case 4:
                    correctButton = chosenButton == pressedPositions[3 - 1];
                    pressedPositions[stage] = pressedPositions[3 - 1];
                    break;
                }
                break;
            }

            if (correctButton)
            {
                stage += 1;
                if (stage >= 4)
                {
                    deactivated = true;
                }
            }
            else
            {
                stage = 0;
                subtractTimer(timer, 15000000, timerMutex);
            }
        }

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            sevenSegment->update();
        }
    }
    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        sevenSegment->setAllDisplay(6, *timer <= 0);
        sevenSegment->setAllDisplay(7, *timer <= 0);
        sevenSegment->update();
    }

    return deactivated;
}

bool lcd_module(Buttons *buttons, Switches *switches, LCD *lcd, int *timer)
{
    unsigned int buttonStates = 0;
    unsigned int switchesStates = 0;
    bool heldButton = false;
    bool deactivated = false;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 7);

    int order = dist(gen);
    std::cout << "lcd: " << order << std::endl;
    std::vector<int> shownCombination;

    for (unsigned int i = 0; i < 10; ++i)
    {
        if (i != (((order - 1) % 10 + 10) % 10) && i != ((order + 5) % 10))
        {
            shownCombination.push_back(i);
        }
    }

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);

    std::shuffle(shownCombination.begin(), shownCombination.end(), rng);

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        lcd->clear();
        for (unsigned int i = 0; i < 8; i++)
        {
            lcd->sendWrite(japaneseSymbols[shownCombination[i]]);
            lcd->sendWrite(" ");
        }

        lcd->update();
    }

    while (!deactivated && *timer > 0)
    {
        buttonStates = buttons->getStatesAsNumber();
        switchesStates = switches->getStatesAsNumber();

        if (buttons->isButtonPressedLong(1, buttonPressDelay))
        {
            heldButton = true;
        }

        // confirmação da escolha
        if (heldButton && !buttons->isButtonPressed(1))
        {
            heldButton = false;
            bool correctCombination = switches->getStatesAsNumber() == (1 << order);

            if (correctCombination)
            {
                deactivated = true;
            }
            else
            {
                subtractTimer(timer, 15000000, timerMutex);
            }
        }
        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            lcd->update();
        }
    }

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        lcd->clear();

        if (*timer <= 0)
        {
            for (unsigned int i = 0; i < 16; i++)
            {
                lcd->sendWrite(defeatSymbol);
            }
        }

        lcd->update();
    }

    return deactivated;
}

int main()
{
    bool restart = true;
    int initialTimerValue = 240;

    while (restart)
    {
        modulesDeactivated = 0;

        std::cout << "Qual o tempo inicial?" << std::endl;
        std::cin >> initialTimerValue;

        if (std::cin.fail())
        {
            std::cerr << "Valor inválido. Iniciando com 240 segundos" << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            initialTimerValue = 240;
        }

        int fileDescriptor = -1;

        fileDescriptor = open("/dev/mydev", O_RDWR);
        if (fileDescriptor < 0)
        {
            std::cerr << "Falha ao abrir dispositivo: " << strerror(errno) << std::endl;
            unsigned int number;
            std::cin >> number;
            return 1;
        }

        Leds redLeds(fileDescriptor, WR_RED_LEDS, 18);
        Leds greenLeds(fileDescriptor, WR_GREEN_LEDS, 8);
        Switches switches(fileDescriptor, RD_SWITCHES, 18);
        Buttons buttons(fileDescriptor, RD_PBUTTONS, 4);
        SevenSegmentDisplays sevenSegment(fileDescriptor, WR_L_DISPLAY, WR_R_DISPLAY, 8);
        LCD lcd(fileDescriptor, WR_LCD_DISPLAY);
        lcd.init();

        int timer = initialTimerValue * 1000000;

#pragma omp parallel sections num_threads(8) shared(timer)
        {
#pragma omp section
            {
                id_module(&sevenSegment);
            }
#pragma omp section
            {
                timer_module(&sevenSegment, &timer);
            }
#pragma omp section
            {
                buttons_module(&buttons, &timer);
            }
#pragma omp section
            {
                switches_module(&switches, &timer);
            }
#pragma omp section
            {
                bool deactivated = red_leds_module(&buttons, &switches, &redLeds, &sevenSegment, &timer);
                if (deactivated)
                {
                    modulesDeactivated++;
                }
            }
#pragma omp section
            {
                bool deactivated = green_leds_module(&buttons, &switches, &greenLeds, &timer);
                if (deactivated)
                {
                    modulesDeactivated++;
                }
            }
#pragma omp section
            {
                bool deactivated = seven_segment_module(&buttons, &switches, &sevenSegment, &timer);
                if (deactivated)
                {
                    modulesDeactivated++;
                }
            }
#pragma omp section
            {
                bool deactivated = lcd_module(&buttons, &switches, &lcd, &timer);
                if (deactivated)
                {
                    modulesDeactivated++;
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            redLeds.setAllStates(timer <= 0);
            greenLeds.setAllStates(timer <= 0);
            sevenSegment.setAllDisplay(4, timer <= 0);
            sevenSegment.setAllDisplay(5, timer <= 0);
            sevenSegment.setAllDisplay(6, timer <= 0);
            sevenSegment.setAllDisplay(7, timer <= 0);
            lcd.clear();

            if (timer <= 0)
            {
                for (unsigned int i = 0; i < 16; i++)
                {
                    lcd.sendWrite(defeatSymbol);
                }
            }

            redLeds.update();
            greenLeds.update();
            sevenSegment.update();
            lcd.update();

            if (fileDescriptor != -1)
            {
                close(fileDescriptor);
            }
        }

        if (timer <= 0)
        {
            std::cout << "Perdestes, a bomba explodiu :<" << std::endl;
        }
        else
        {
            std::cout << "Ganhastes, você salvou a todos!!!" << std::endl;
        }

        std::cout << "Deseja recomeçar? (s/n): ";
        char choice;
        std::cin >> choice;

        if (choice != 's' && choice != 'S')
        {
            restart = false;
        }
    }

    return 0;
}