#ifndef VM_H
#define VM_H 1

#include <vector>
#include <unordered_map>

using namespace std;

typedef enum {
    OP_LA,   // Load Address:
    OP_LV,   // Load Value:
    OP_LC,   // load Constant
    OP_LI,   // Load Indirect
    OP_INT,  // Increment t
    OP_DCT,  // Decrement t
    OP_J,    // Jump
    OP_FJ,   // False Jump
    OP_HL,   // Halt
    OP_ST,   // Store
    OP_CALL, // Call
    OP_EP,   // Exit Procedure
    OP_EF,   // Exit Function
    OP_RC,   // Read Char
    OP_RI,   // Read Integer
    OP_WRC,  // Write Char
    OP_WRI,  // Write Int
    OP_WLN,  // WriteLN
    OP_ADD,  // Add
    OP_SUB,  // Substract
    OP_MUL,  // Multiple
    OP_DIV,  // Divide
    OP_MOD,  // Module
    OP_NEG,  // Negative
    OP_CV,   // Copy Top
    OP_ODD,  // Odd
    OP_EQ,   // Equal
    OP_NE,   // Not Equal
    OP_GT,   // Greater
    OP_LT,   // Less
    OP_GE,   // Greater or Equal
    OP_LE,   // Less or Equal
    OP_BP    // Break point.
} OpCode ;

struct Instruction {
    OpCode Op;
    int p;
    int q;
};

class VM
{
private:
    unordered_map<string, OpCode> toOpCode = {
        {"la", OP_LA},   // Load Address:
        {"lv", OP_LV},   // Load Value:
        {"lc", OP_LC},   // load Constant
        {"li", OP_LI},   // Load Indirect
        {"int", OP_INT},  // Increment t
        {"dct", OP_DCT},  // Decrement t
        {"j", OP_J},    // Jump
        {"fj", OP_FJ},   // False Jump
        {"hlt", OP_HL},   // Halt
        {"st", OP_ST},   // Store
        {"call", OP_CALL}, // Call
        {"ep", OP_EP},   // Exit Procedure
        {"ef", OP_EF},   // Exit Function
        {"rc", OP_RC},   // Read Char
        {"ri", OP_RI},   // Read Integer
        {"wrc", OP_WRC},  // Write Char
        {"wri", OP_WRI},  // Write Int
        {"wln", OP_WLN},  // WriteLN
        {"add", OP_ADD},  // Add
        {"sub", OP_SUB},  // Substract
        {"mul", OP_MUL},  // Multiple
        {"div", OP_DIV},  // Divide
        {"mod", OP_MOD},  // Module
        {"neg", OP_NEG},  // Negative
        {"cv", OP_CV},   // Copy Top
        {"odd", OP_ODD}, // Odd
        {"eq", OP_EQ},   // Equal
        {"ne", OP_NE},   // Not Equal
        {"gt", OP_GT},   // Greater
        {"lt", OP_LT},   // Less
        {"ge", OP_GE},   // Greater or Equal
        {"le", OP_LE},   // Less or Equal
        {"bp", OP_BP}    // Break point.
    };

    vector<Instruction> code;
    vector<int> stk;
    int PC, B, T;

    void pushStack(int val);

    void call();
    void ep();
    void ef();

    void writec();
    void writei();
    void writeln();

    void readc();
    void readi();

    void breakPoint();

    int base(int L);
    int loadCode(const char* input);
public:
    void intepreter(const char* input);
};

#endif // VIRTUAL_MACHINE
