#include "Library.h"
#include <iostream>

void readln(CodeGenerator& codeGenerator)
{
    codeGenerator.gen(OP_INT, 1 + 4);
    codeGenerator.gen(OP_RI);
    codeGenerator.gen(OP_EP);
}

void writeln(CodeGenerator& codeGenerator)
{
    codeGenerator.gen(OP_INT, 1 + 4);
    codeGenerator.gen(OP_WRI);
    codeGenerator.gen(OP_WLN);
    codeGenerator.gen(OP_EP);
}

Builtin::Builtin()
{
    ProcInfo info;
    ProcEntry entry;
    vector<bool> argv;
    entry.kind = Object::PROCEDURE;
    entry.argc = 1;
    argv.push_back(true);
    entry.argv = argv;
    info.entry = entry;
    info.p = &readln;

    builtin.insert({new string("readln"), info});

    info.p = &writeln;
    info.entry.argv[0] = false;
    builtin.insert({new string("writeln"), info});
}

ProcInfo Builtin::get(string* name)
{
    try {
        if(builtin.find(name) == builtin.end()) {
            throw "Built-in function not exist.";
        }
        return builtin[name];
    } catch(const char* msg) {
        cerr << msg << endl;
    }
}

bool Builtin::isExist(string* name)
{
    return builtin.find(name) != builtin.end();
}
