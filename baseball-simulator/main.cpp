//First version of main.cpp that will be the highest level operation of the simulator
//
//Main
//
//JEY Solutions
//
//Created By: Yoni Arieh
//5/20/21
//

//Standard Includes
#include <stdio.h>
#include <iostream>

//Custom JEY Solutions headers
#include "BaseballSimulator.h"
#include <string>

int main(){
    BaseballSimulator system;
    while(true){
        std::cout << "Enter 0 for MainTestingRoutine, 1 for savingRoutine, 2 for loadingRoutine, 3 for photographyRoutine, 4 for distFromWall, 5 for singleMeasurement\n";
        int choice=0;
        std::cin >> choice;
        
        if (choice == 0){
            std::cout << "Running MainTestingRoutine\n";
            system.MainTestingRoutine();
        }
        else if (choice == 1){
            std::cout << "Running savingRoutine\n";
            std::cout << "Enter string for name of file to save (include folder name)\n";
            std::string str;
            std:: cin >> str;
            system.savingRoutine(str);
        }
        else if (choice == 2){
            std::cout << "Running loadingRoutine\n";
            std::cout << "Enter string for name of file to load (include folder name)\n";
            std::string str;
            std:: cin >> str;
            system.loadingRoutine(str);
        }
        else if (choice == 3){
            std::cout << "Running photographyRoutine\n";
            system.photographyRoutine();
        }
        else if (choice == 4){
            std::cout << "Running distFromWall\n";
            system.distFromWall();
        }
        else if (choice == 5){
            std::cout << "singleMeasurement\n";
            system.singleMeasurement();
        }
    }
}

