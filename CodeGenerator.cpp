#include "CodeGenerator.h"
#include <iostream>

using namespace std;

CodeGenerator::CodeGenerator()
{
}

int CodeGenerator::lvalue(VarEntry* entry, int base)
{
    Addr addr = entry->addr;
    if(entry->PassByValue) {
        gen(OP_LV, base - addr.base, addr.offset);
    } else {
        gen(OP_LV, base - addr.base, addr.offset);
        gen(OP_LI);
    }
}

void CodeGenerator::gen(OpCode opCode)
{
    Instruction i;
    i.Op = opCode;
    code.push_back(i);
}

void CodeGenerator::gen(OpCode opCode, int p)
{
    Instruction i;
    i.Op = opCode;
    i.p = p;
    code.push_back(i);
}

void CodeGenerator::gen(OpCode opCode, int p, int q)
{
    Instruction i;
    i.Op = opCode;
    i.p = p;
    i.q = q;
    code.push_back(i);
}

int CodeGenerator::getCodeSize()
{
    return code.size();
}

void CodeGenerator::setP(int pos, int p)
{
    code[pos].p = p;
}

void CodeGenerator::setOp(int pos, OpCode op)
{
    code[pos].Op = op;
}

int CodeGenerator::save(const char* output)
{
    FILE *f = fopen(output, "wt");
    int len = code.size();
    for(int i = 0; i < len; i++) {
        switch(code[i].Op) {
        case OP_LA:   // Load Address:
        case OP_LV:   // Load Value:
        case OP_CALL: // Call
            fprintf(f, "%s %d, %d\n", toString[code[i].Op], code[i].p, code[i].q);
            break;
        case OP_LC:   // load Constant
        case OP_INT:  // Increment t
        case OP_DCT:  // Decrement t
        case OP_J:    // Jump
        case OP_FJ:   // False Jump
            fprintf(f, "%s %d\n", toString[code[i].Op], code[i].p);
            break;
        case OP_HL:   // Halt
        case OP_ST:   // Store
        case OP_EP:   // Exit Procedure
        case OP_EF:   // Exit Function
        case OP_RC:   // Read Char
        case OP_RI:   // Read Integer
        case OP_WRC:  // Write Char
        case OP_WRI:  // Write Int
        case OP_WLN:  // WriteLN
        case OP_ADD:  // Add
        case OP_SUB:  // Substract
        case OP_MUL:  // Multiple
        case OP_DIV:  // Divide
        case OP_MOD:  // Module
        case OP_NEG:  // Negative
        case OP_CV:   // Copy Top
        case OP_ODD:  // Odd
        case OP_EQ:   // Equal
        case OP_NE:   // Not Equal
        case OP_GT:   // Greater
        case OP_LT:   // Less
        case OP_GE:   // Greater or Equal
        case OP_LE:   // Less or Equal
        case OP_BP:    // Break point.
        case OP_LI:   // Load Indirect
            fprintf(f, "%s\n", toString[code[i].Op]);
            break;
        }
    }

    fclose(f);
}
