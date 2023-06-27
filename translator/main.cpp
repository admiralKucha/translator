#include <iostream>
#include <fstream>
#include "Scanner.h"
#include "parser.h"
#include <locale.h>


int main() 
{
    setlocale(LC_CTYPE, "rus");
    std::ifstream ifile("MiniCProgramm.txt");
    try 
    {
        Parser p{ ifile };
        p.StmtList(GlobalScope);
        p.checkMain();
        p.printAtoms(std::cout);
        p.printSymbolTable(std::cout);
    }
    catch (const std::exception& error) {
        std::cout << error.what() << std::endl;
    }
    return 0;
}