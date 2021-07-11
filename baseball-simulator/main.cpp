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

int main(){
    BaseballSimulator system;
    while(true){
        char yesNo;
        std::cout << "Run System Process? (Y/N)" << std::endl;
        std::cin >> yesNo;
        if (yesNo == 'N')
            break;
        system.Save();
    }
}

