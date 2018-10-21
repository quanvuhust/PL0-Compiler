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
    parser.parse("./test/test00.pl0");
    VM vm;
    vm.intepreter("./test/test00.out");
    return 0;
}
