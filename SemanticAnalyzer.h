#ifndef SEMANTIC_ANALYZER
#define SEMANTIC_ANALYZER 0
#include <unordered_map>
#include <iostream>
#include <string>
#include "LexicalAnalyzer.h"

typedef char ObjectType;

class Object
{
public:
    static const ObjectType CONST=0, VARIABLE=1, PROCEDURE=2, ARRAY=3;
};

class Entry
{
public:
    ObjectType kind;
};


struct Addr {
    int base;
    int offset;
};

class ConstEntry:public Entry
{
public:
    int val;
};

class VarEntry:public Entry
{
public:
    bool PassByValue = true;
    Addr addr;
};

class ArrayEntry:public Entry
{
public:
    Addr addr;
    int len;
};

class ProcEntry:public Entry
{
public:
    void *childProc;
    int argc;
    int base;
    int address;
    // Tham bien: true
    // Tham tri: false
    vector<bool> argv;
};

struct myhash {
    std::size_t operator() (const std::string *key) const
    {
        return std::hash<std::string>()(*key);
    }
};

struct myequal {
public:
    bool operator()(const std::string *val1, const std::string *val2) const
    {
        return *val1 == *val2;
    }
};

class SymbolTable
{
public:
    int width = 0;
    SymbolTable *parent = nullptr;
    std::unordered_map<std::string*, void*, myhash, myequal> table;

    ~SymbolTable()
    {
        for (auto& it: table) {
            delete it.first;
            ObjectType kind = ((Entry*)it.second)->kind;
            if(kind == Object::ARRAY) {
                delete (ArrayEntry*) it.second;
            } else if(kind == Object::VARIABLE) {
                delete (VarEntry*) it.second;
            } else if(kind == Object::CONST) {
                delete (ConstEntry*) it.second;
            } else if(kind == Object::PROCEDURE) {
                delete (ProcEntry*) it.second;
            }
        }
    }
};

class SemanticAnalyzer
{
private:
    unordered_map<int, const char*> errorString = {{25, "Redeclaration of "},
        {26, "\' was not declared in this scope"}, {27, "CONST can not be assigned."}, {28, "PROCEDURE can not be assigned."},
        {29, "Procedure cannot be in Expression."}, {30, "Const ARRAY variable can not be assigned."},
        {31, "Missing argument of procedure: "}, {32, "Too many argument of procedure: "},
        {33, "VARIABLE or CONST cannot be referenced by index array"}, {34, "\' is a built-in function"},
        {35, "Missing array index"}, {36, "Tham bien khong the la hang so, thu tuc hoac bi de trong"}, {37, "\' is not a PROCEDURE"}, 
        {38, "IDENT in FOR loop must be a VARIABLE."}, {39, "Tham bien khong the bi de trong."}
    };
public:
    bool checkIdent(SymbolTable *table, std::string* name);
    void* getEntry(SymbolTable *table, std::string* name);
    SymbolTable* mktable(SymbolTable *prevTable);

    SymbolTable* enterProc(std::string* name, int base, SymbolTable *table);
    void enterConst(std::string* name, SymbolTable *table, NumberType val);
    void enterVar(std::string* name, Addr addr, SymbolTable *table);
    void enterVar(std::string* name, Addr addr, bool passBy, SymbolTable *table);
    void enterArray(std::string* name, Addr addr, SymbolTable *table, int len);

    void setParameterProc(SymbolTable *table, string *nameProc, int argc, vector<bool> &argv, int address);
    int checkParameterProc(SymbolTable *table, string *nameProc, int argc);
    void error(int errorCode, string name="");
};

#endif // SEMANTIC_ANALYZER
