#include "VM.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int VM::base(int L)
{
    int c = B;
    while (L > 0) {
        c = stk[c + 3];
        L--;
    }
    return c;
}

int VM::loadCode(const char* input)
{
    ifstream ifs;
    ifs.open(input, ios::in);
    if (ifs.is_open()) {
        string line;
        string token;
        while(getline(ifs, line)) {
            if(line == "") {
                continue;
            }
            int c = 0;
            Instruction i;
            istringstream iss(line);
            getline(iss, token, ' ');
            for(int i = 0; i < token.length(); ++i) {
                token[i] = tolower(token[i]);
            }
            if(toOpCode.find(token) != toOpCode.end()) {
                i.Op = toOpCode[token];
            } else {
                cerr << "Op " << token << " does not exist." << endl;
                ifs.close();
                return -1;
            }

            while (getline(iss, token, ',')) {
                if(c == 0) {
                    i.p = stoi(token);
                    c++;
                } else if(c == 1) {
                    i.q = stoi(token);
                    c++;
                } else {
                    cerr << "Op " << token << " have too many operand." << endl;
                    ifs.close();
                    return -1;
                }
            }
            code.push_back(i);
        }
    } else {
        cerr << "File " << input << " does not exist." << endl;
        return -1;
    }

    ifs.close();
    return 0;
}

void VM::call()
{
    int x = (T + 4 + 1) - stk.size();
    while(x < 0) {
        stk.push_back(x++);
    }
    stk[T+2] = B;                 // Dynamic Link
    stk[T+3] = PC;                // Return Address
    stk[T+4] = base(code[PC].p);  // Static Link
    B = T + 1;                      // Base & Result
    PC = code[PC].q - 1;
}

void VM::ep()
{
    T = B - 1;
    PC = stk[B + 2];
    B = stk[B + 1];
}

void VM::ef()
{
    T = B;
    PC = stk[B + 2];
    B = stk[B + 1];
}

void VM::writec()
{
    printf("%c", stk[T--]);
}

void VM::writei()
{
    printf("%d", stk[T--]);
}

void VM::writeln()
{
    cout << endl;
}

void VM::readc()
{
    char c;
    scanf("%c", &c);
    stk[stk[T--]] = c;
}

void VM::readi()
{
    int x;
    scanf("%d", &x);
    stk[stk[T--]] = x;
}

void VM::breakPoint()
{

}

void VM::pushStack(int val)
{
    T++;
    if(T == stk.size()) {
        stk.push_back(val);
    } else {
        stk[T] = val;
    }
}

void VM::intepreter(const char* input)
{
    PC = 0;
    T = -1;
    B = 0;
    if(loadCode(input) == -1) {
        return;
    }
    int exit = 0;
    do {
       switch (code[PC].Op) {
        case OP_LA:
            pushStack(base(code[PC].p) + code[PC].q);
            break;
        case OP_LI:
            stk[T] = stk[stk[T]];
            break;
        case OP_LV:
            pushStack(stk[base(code[PC].p) + code[PC].q]);
            break;
        case OP_LC:
            pushStack(code[PC].p);
            break;
        case OP_FJ:
            if(stk[T--] == 0) {
                PC = code[PC].p - 1;
            }
            break;
        case OP_HL:
            exit = 1;
            break;
        case OP_ST:
            stk[stk[T-1]] = stk[T];
            T -= 2;
            break;
        case OP_INT:
            T += code[PC].p;
            if(T >= stk.size()) {
                stk.resize(T + 1);
            }
            break;
        case OP_DCT:
            T -= code[PC].p;
            break;
        case OP_CALL:
            call();
            break;
        case OP_J:
            PC = code[PC].p - 1; // Sau khi thuc hien cau lenh PC se tang len 1 don vi
            break;
        case OP_EP:
            ep();
            break;
        case OP_EF:
            ef();
            break;
        case OP_RC:
            readc();
            break;
        case OP_RI:
            readi();
            break;
        case OP_WRC:
            writec();
            break;
        case OP_WRI:
            writei();
            break;
        case OP_WLN:
            writeln();
            break;
        case OP_ADD:
            T--;
            stk[T] = stk[T] + stk[T + 1];
            break;
        case OP_SUB:
            T--;
            stk[T] = stk[T] - stk[T + 1];
            break;
        case OP_MUL:
            T--;
            stk[T] = stk[T] * stk[T + 1];
            break;
        case OP_DIV:
            T--;
            stk[T] = stk[T] / stk[T + 1];
            break;
        case OP_MOD:
            T--;
            stk[T] = stk[T] % stk[T + 1];
            break;
        case OP_NEG:
            stk[T] = -stk[T];
            break;
        case OP_CV:
            pushStack(stk[T]);
            break;
        case OP_ODD:
            stk[T] &= 1;
            break;
        case OP_EQ:
            T--;
            if(stk[T] == stk[T+1]) {
                stk[T] = 1;
            } else {
                stk[T] = 0;
            }
            break;
        case OP_NE:
            T--;
            if(stk[T] != stk[T+1]) {
                stk[T] = 1;
            } else {
                stk[T] = 0;
            }
            break;
        case OP_GT:
            T--;
            if(stk[T] > stk[T+1]) {
                stk[T] = 1;
            } else {
                stk[T] = 0;
            }
            break;
        case OP_LT:
            T--;
            if(stk[T] < stk[T+1]) {
                stk[T] = 1;
            } else {
                stk[T] = 0;
            }
            break;
        case OP_GE:
            T--;
            if(stk[T] >= stk[T+1]) {
                stk[T] = 1;
            } else {
                stk[T] = 0;
            }
            break;
        case OP_LE:
            T--;
            if(stk[T] <= stk[T+1]) {
                stk[T] = 1;
            } else {
                stk[T] = 0;
            }
            break;
        case OP_BP:
            breakPoint();
            break;
        } //switch
        PC++;
    } while(exit == 0);
}//intepreter
