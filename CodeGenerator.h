#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include "VM.h"
#include "SemanticAnalyzer.h"

class CodeGenerator
{
public:
    CodeGenerator();
    void rvalue(VarEntry* entry, int base);
    void lvalue(VarEntry* entry, int base);
    void gen(OpCode opCode);
    void gen(OpCode opCode, int p);
    void gen(OpCode opCode, int p, int q);
    int getCodeSize();
    void setP(int pos, int p);
    void setOp(int pos, OpCode op);
    int save(const char* output);
private:
    vector<Instruction> code;
    unordered_map<int, const char*> toString = {
        {OP_LA, "la"},   // Load Address:
        {OP_LV, "lv"},   // Load Value:
        {OP_LC, "lc"},   // load Constant
        {OP_LI, "li"},   // Load Indirect
        {OP_INT, "int"},  // Increment t
        {OP_DCT, "dct" },  // Decrement t
        {OP_J, "j"},    // Jump
        {OP_FJ, "fj"},   // False Jump
        {OP_HL, "hlt"},   // Halt
        {OP_ST, "st"},   // Store
        {OP_CALL,"call" }, // Call
        {OP_EP,"ep" },   // Exit Procedure
        {OP_EF,"ef"},   // Exit Function
        {OP_RC,"rc" },   // Read Char
        {OP_RI,"ri" },   // Read Integer
        {OP_WRC,"wrc" },  // Write Char
        {OP_WRI,"wri" },  // Write Int
        {OP_WLN,"wln" },  // WriteLN
        {OP_ADD,"add" },  // Add
        {OP_SUB,"sub" },  // Substract
        {OP_MUL,"mul" },  // Multiple
        {OP_DIV,"div" },  // Divide
        {OP_MOD,"mod" },  // Module
        {OP_NEG,"neg" },  // Negative
        {OP_CV,"cv" },   // Copy Top
        {OP_ODD,"odd" }, // Odd
        {OP_EQ,"eq"},   // Equal
        {OP_NE,"ne"},   // Not Equal
        {OP_GT,"gt"},   // Greater
        {OP_LT,"lt"},   // Less
        {OP_GE,"ge"},   // Greater or Equal
        {OP_LE,"le"},   // Less or Equal
        {OP_BP,"bp"}    // Break point.
    };
};

#endif // CODEGENERATOR_H
