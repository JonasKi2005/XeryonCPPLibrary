#include "Xeryon.hpp"
#include <iostream>
using namespace Xeryon;

int main()
{
    Xeryon::Controller *controller = new Xeryon::Controller("/dev/ttyACM0", 9600, true, "settings_default.txt");
    Xeryon::Axis *A = controller->addAxis(Stage::create(Stage::Type::XLS_312_3N), 'A');
    Xeryon::Axis *B = controller->addAxis(Stage::create(Stage::Type::XLS_312_3N), 'B');
    Xeryon::Axis *C = controller->addAxis(Stage::create(Stage::Type::XLS_78_3N), 'C');
    Xeryon::Axis *D = controller->addAxis(Stage::create(Stage::Type::XLS_78_3N), 'D');
    Xeryon::Axis *E = controller->addAxis(Stage::create(Stage::Type::XLS_78_3N), 'E');

    controller->start();
    A->findIndex();
    B->findIndex();
    C->findIndex();
    D->findIndex();
    E->findIndex();

    std::cout << "Found all indicies!!";
    A->setDPOS(5_mm);

    controller->stop();
    delete controller;
    return EXIT_SUCCESS;
}