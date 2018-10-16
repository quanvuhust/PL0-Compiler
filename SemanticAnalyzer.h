#ifndef SEMANTIC_ANALYZER
#define SEMANTIC_ANALYZER 0
#include <unordered_map>
#include <string>
#include "LexicalAnalyzer.h"
#include "Library.h"

typedef int ObjectType;

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

class ConstEntry:public Entry
{
    public:
    int val;
};

class VarEntry:public Entry
{
    public:
};

class ArrayEntry:public Entry
{
    public:
    int len;
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
    SymbolTable *parent = nullptr;
    std::unordered_map<std::string*, void*, myhash, myequal> table;
    void removeAllEntry(void) {
        for (auto& it: table) {
            delete it.first;
            delete it.second;
        }
    }
};

class ProcEntry:public Entry
{
    public:
    SymbolTable *childProc;
    int argc;
    vector<string*> argv;
};

class SemanticAnalyzer
{
public:
    bool checkIdent(SymbolTable *table, std::string* name);
    void* getEntry(SymbolTable *table, std::string* name);
    SymbolTable* mktable(SymbolTable *prevTable);

    SymbolTable* enterProc(std::string* name, SymbolTable *table);
    void enterConst(std::string* name, SymbolTable *table, NumberType val);
    void enterVar(std::string* name, SymbolTable *table);
    void enterArray(std::string* name, SymbolTable *table, int len);

    void setParameterProc(SymbolTable *table, string *nameProc, int argc, vector<string*> &argv);
    int checkParameterProc(SymbolTable *table, string *nameProc, int argc);
};

#endif // SEMANTIC_ANALYZER
