#ifndef LIBRARY_H
#define LIBRARY_H 1

#include <unordered_map>
#include <vector>
#include "SemanticAnalyzer.h"
#include "CodeGenerator.h"

using namespace std;

struct ProcInfo {
    ProcEntry entry;
    void(*p)(CodeGenerator&);
};

class Builtin {
    private:
    unordered_map<string*, ProcInfo, myhash, myequal> builtin;

    public:
    Builtin();
    ProcInfo get(string* name);
    bool isExist(string* name);
};


#endif
