#include "SemanticAnalyzer.h"
#include <iostream>
#include <cassert>

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

void SemanticAnalyzer::enterVar(string* name, SymbolTable *table)
{
    assert(table != nullptr);
    VarEntry *entry = new VarEntry();
    entry->kind = Object::VARIABLE;
    table->table.insert({name, entry});
}

void SemanticAnalyzer::enterArray(std::string* name, SymbolTable *table, int len)
{
    assert(table != nullptr);
    ArrayEntry *entry = new ArrayEntry();
    entry->kind = Object::ARRAY;
    entry->len = len;
    table->table.insert({name, entry});
}

SymbolTable* SemanticAnalyzer::enterProc(string* name, SymbolTable *table)
{
    assert(table != nullptr);
    SymbolTable *newTable = new SymbolTable();
    assert(newTable != nullptr);

    newTable->parent = table;
    ProcEntry *entry = new ProcEntry();
    entry->childProc = newTable;
    entry->kind = Object::PROCEDURE;
    table->table.insert({name, entry});
    return newTable;
}

void* SemanticAnalyzer::getEntry(SymbolTable *table, string* name)
{
    if(table == nullptr) {
        return nullptr;
    }

    if(table->table.find(name) != table->table.end()) {
        return table->table[name];
    }

    return getEntry(table->parent, name);
}

void SemanticAnalyzer::setParameterProc(SymbolTable *table, string *nameProc, int argc, vector<string*> &argv)
{
    ProcEntry* entry = (ProcEntry*)getEntry(table, nameProc);
    if(entry != nullptr) {
        entry->argc = argc;
        entry->argv = argv;
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
