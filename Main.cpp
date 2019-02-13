#include <iostream>
#include <unordered_map>
#include "Parser.h"
#include "VM.h"

using namespace std;

int main(int argc, char* argv[]) {

    /*
    if(argc < 2) {
        printf("Missing argument.\n");
        getchar();
        return 1;
    }*/
    Parser parser;
    if(!parser.parse("./test/test01.pl0")) {
        VM vm;
        vm.intepreter("./test/test01.out");
    }

    return 0;
}
