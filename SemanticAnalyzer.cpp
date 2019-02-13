#include "SemanticAnalyzer.h"
#include "Parser.h"

#include <iostream>
#include <cassert>

void SemanticAnalyzer::error(int errorCode, string name)
{
    if(errorCode == 26 || errorCode == 34 || errorCode == 37) {
        cerr << "\'" << name << errorString[errorCode] << endl;
    } else {
        cerr << errorString[errorCode] << name << endl;
    }
}

bool SemanticAnalyzer::checkIdent(SymbolTable *table, string* name)
{
    assert(table != nullptr);
    if(table->table.find(name) != table->table.end()) {
        return true;
    }

    return false;
}

void SemanticAnalyzer::enterConst(string* name, SymbolTable *table, NumberType val)
{
    assert(table != nullptr);
    ConstEntry *entry = new ConstEntry();
    entry->kind = Object::CONST;
    entry->val = val;
    table->table.insert({name, entry});
}

void SemanticAnalyzer::enterVar(string* name, Addr addr, SymbolTable *table)
{
    assert(table != nullptr);
    VarEntry *entry = new VarEntry();
    entry->kind = Object::VARIABLE;
    entry->addr = addr;
    table->table.insert({name, entry});
    table->width += 1;
}

void SemanticAnalyzer::enterVar(std::string* name, Addr addr, bool passBy, SymbolTable *table)
{
    assert(table != nullptr);
    VarEntry *entry = new VarEntry();
    entry->kind = Object::VARIABLE;
    entry->addr = addr;
    entry->PassByValue = passBy;
    table->table.insert({name, entry});
    table->width += 1;
}

void SemanticAnalyzer::enterArray(std::string* name, Addr addr, SymbolTable *table, int len)
{
    assert(table != nullptr);
    ArrayEntry *entry = new ArrayEntry();
    entry->kind = Object::ARRAY;
    entry->addr = addr;
    entry->len = len;
    table->table.insert({name, entry});
    table->width += len;
}

SymbolTable* SemanticAnalyzer::enterProc(string* name, int base, SymbolTable *table)
{
    assert(table != nullptr);
    SymbolTable *newTable = new SymbolTable();
    assert(newTable != nullptr);

    newTable->parent = table;
    ProcEntry *entry = new ProcEntry();
    entry->childProc = newTable;
    entry->kind = Object::PROCEDURE;
    entry->base = base;
    if(table->table.find(name) != table->table.end()) {
        table->table.erase(name);
    }
    table->table.insert({name, entry});
    return newTable;
}

void* SemanticAnalyzer::getEntry(SymbolTable *table, string* name)
{
    while(table != nullptr) {
        if(table->table.find(name) != table->table.end()) {
            return table->table[name];
        }
        table = table->parent;
    }
    return nullptr;
}

void SemanticAnalyzer::setParameterProc(SymbolTable *table, string *nameProc, int argc, vector<bool> &argv, int address)
{
    ProcEntry* entry = (ProcEntry*)getEntry(table, nameProc);
    if(entry != nullptr) {
        entry->argc = argc;
        entry->argv = argv;
        entry->address = address;
    }
}

int SemanticAnalyzer::checkParameterProc(SymbolTable *table, string *nameProc, int argc)
{
    assert(table != nullptr);
    ProcEntry* entry = (ProcEntry*)getEntry(table, nameProc);
    if(entry == nullptr) {
        return 0;
    }
    if(argc < entry->argc) {
        return -1;
    } else if(entry->argc == argc) {
        return 0;
    } else {
        return 1;
    }
}

SymbolTable* SemanticAnalyzer::mktable(SymbolTable *prevTable)
{
    SymbolTable* newTable = new SymbolTable();
    assert(newTable != nullptr);
    newTable->parent = prevTable;
    return newTable;
}
