#include <iostream>
#include "Program.h"
#include <conio.h>
using namespace std;

Program::Program()
{
    this->Vol = nullptr;
}

Program::~Program()
{
    if (this->Vol != nullptr)
    {
        delete this->Vol;
        this->Vol = nullptr;
    }
}

void Program::run()
{
    this->homeNavigate();
}

void Program::homeNavigate()
{
    system("cls");
    this->printHomeScreen();
    bool back = false;
    char choice;
    while (true)
    {
        cout << "ENTER YOUR CHOICE: " << endl;
        cin >> choice;
        if (isdigit(choice))
        {
            break;
        }
        cout << "PLEASE ENTER A NUMBER" << endl;
    }
    switch (choice)
    {
    case '1':
        this->createVolume();
        break;
    case '2':
        this->openVolume();
        break;
    case '3':
        break;
    case '4':
        break;
    case '5':
        break;
    case '6':
        break;
    case '7':
        break;
    }
}

void Program::createVolume()
{
    system("cls");
    cout << "===== CREATE A VOLUME =====" << endl
         << endl;

    // Input a path
    cout << "  Program: * Input a path to create a volume"
         << "\n\n";
    cout << "           * Do not input anything then press B to EXIT" << endl
         << endl;
    cout << "  User: ";

    string volumeFilePath;
    getline(cin, volumeFilePath);
    // If input is ""
    if (volumeFilePath == "")
    {
        return;
    }
    // Create this volume
    this->initializeVolume(volumeFilePath + Volume::Extension);
    // If volume already existed
    if (this->Vol->create())
    {
        this->Cache.add(this->Vol->getPath());
        cout << "\n";
        cout << "  Program: * The new volume is created successfully!"
             << "\n\n";
        cout << "           * Press Enter to";
        cout << " OPEN ";
        cout << "this volume"
             << "\n\n";
        cout << "           * Press any key except for Enter to";
        cout << " EXIT";
        cout << "\n\n";
        while (true)
        {
            if (_kbhit())
            {
                FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
                if ((GetKeyState(VK_RETURN) & 0x8000))
                {
                    system("cls");
                    this->Vol->open();
                    break;
                }
                else
                    break;
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        cout << endl;
        cout << "  Program: Cannot create a new volume."
             << "\n\n";
        cout << "           This path does not exist OR it is taken."
             << "\n\n";
        cout << "           Please input another path."
             << "\n\n";
        cout << "  ";
        system("pause");
    }
    this->closeVolume();
}

void Program::initializeVolume(string const &volumeFilePath)
{
    this->Vol = new Volume(volumeFilePath);
}

void Program::closeVolume()
{
    delete this->Vol;
    this->Vol = nullptr;
}

void Program::printHomeScreen()
{
    cout << "// /////////////////////////////////////////////" << endl;
    cout << "// University of Science - Ho Chi Minh City" << endl;
    cout << "// fit - Operating System" << endl;
    cout << "// Project 3.4.";
    cout << "// From: ";
    cout << "//       Ly Anh Quan - 18127017";
    cout << "//       Luong Xuan Thanh - 18127259";
    cout << "//       Lam Gia Lac - 18127268";
    cout << "//";
    cout << " .================================================." << endl;
    cout << "|| 1.> Create a new volume                        ||" << endl;
    cout << "|| 2.> Open an existing volume                    ||" << endl;
    cout << "|| 3.> Set password                               ||" << endl;
    cout << "|| 4.> Import Volume                              ||" << endl;
    cout << "|| 5.> Outport Volume                             ||" << endl;
    cout << "|| 6.> Delete Volume                              ||" << endl;
    cout << "|| 7.> Exit                                       ||" << endl;
    cout << " *================================================*" << endl;
}