#include "Parser.h"
#include "LexicalAnalyzer.h"
#include <iostream>
#include <set>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cassert>
#include <cstring>

using namespace std;

Flag flag;

void Parser::syntaxError(int errorCode, string name)
{
    flag.numError++;

    cerr << "Line " << lexicalAnalyzer.getLine() << ": "
         << "Error code " << errorCode << ": ";
    syntaxAnalyzer.error(errorCode, name);
}

void Parser::semanticError(int errorCode, string name)
{
    flag.numError++;
    cerr << "Line " << lexicalAnalyzer.getLine() << ": "
         << "Error code " << errorCode << ": ";
    semanticAnalyzer.error(errorCode, name);
}


void Parser::nextToken(void)
{
    if (lexicalAnalyzer.hasNextToken()) {
        token = lexicalAnalyzer.getNextToken();
    }
}

void Parser::loadOldTable()
{
    delete table;
    table = tblptr.back();
    tblptr.pop_back();
    base--;
}

void Parser::declareVariable()
{
    string* id = nullptr;
    Addr addr;
    if (token == IDENT) {
        // Kiem tra khai bao bien
        id = lexicalAnalyzer.getIdent();
        if (semanticAnalyzer.checkIdent(table, id)) {
            semanticError(25, *id); // 25, "Redeclaration of "
        }

        addr.base = base;
        addr.offset = offset;
        nextToken();
    } else {
        syntaxError(18); // 18, "Missing variable name."
    }

    if (token == LBRACK) {
        nextToken();
        if (token == NUMBER) {
            if (id != nullptr) {
                int len = (int)lexicalAnalyzer.getNumber();
                semanticAnalyzer.enterArray(id, addr, table, len);
                offset += len;
            }
            nextToken();
        } else {
            syntaxError(19); // 19, "Missing array length."
        }
        if (token == RBRACK) {
            nextToken();
        } else {
            syntaxError(0); // 0, "Missing ]"
        }
    } else {
        if (id != nullptr) {
            semanticAnalyzer.enterVar(id, addr, table);
            offset++;
        }
    }
}

void Parser::declareConst()
{
    string* id = nullptr;
    if (token == IDENT) {
        id = lexicalAnalyzer.getIdent();
        if (semanticAnalyzer.checkIdent(table, id)) {
            semanticError(25, *id); // 25, "Redeclaration of "
        } 

        nextToken();
    } else {
        syntaxError(22); // 22, "Missing const name."
    }

    if (token == EQU) {
        nextToken();
    } else {
        syntaxError(16); // 16, "Missing operator \'=\'"
    }

    if (token == NUMBER) {
        if (id != nullptr) {
            semanticAnalyzer.enterConst(id, table, lexicalAnalyzer.getNumber());
        }
        nextToken();
    } else {
        syntaxError(17); // 17, "Missing number when assign a const"
    }
}

void Parser::declareProc()
{
    string* id = nullptr;
    string* nameProc = nullptr;
    int pos = 0;

    if (token == IDENT) {
        // Kiem tra khai bao procedure
        id = lexicalAnalyzer.getIdent();
        nameProc = id;

        if (builtin.isExist(id)) {
            nameProc = nullptr;
            semanticError(34, *id); // 34, "\' is a built-in function"
        } else if (semanticAnalyzer.checkIdent(table, id)) {
            nameProc = nullptr;
            semanticError(25, *id); // 25, "Redeclaration of "
        }
        nextToken();
    } else {
        syntaxError(20); // 20, "Missing procedure name."
    }

    tblptr.push_back(table);
    pos = codeGenerator.getCodeSize();
    codeGenerator.gen(OP_J, 0);

    if (nameProc != nullptr) {
        table = semanticAnalyzer.enterProc(nameProc, base, table);
    } else {
        table = semanticAnalyzer.enterProc(new string("0_anonymous"), base, table);
    }

    base++;
    offset = 4;

    int argc = 0;
    vector<bool> argv;

    if (token == LPARENT) {
        do {
            nextToken();
            argv.push_back(false);
            if (token == VAR) {
                argv.back() = true;
                nextToken();
            }
            if (token == IDENT) {
                Addr addr;
                addr.base = base;
                addr.offset = offset++;
                argc++;
                id = lexicalAnalyzer.getIdent();
                if (semanticAnalyzer.checkIdent(table, id)) {
                    semanticError(25, *id); // 25, "Redeclaration of "
                }

                if (argv.back()) {
                    semanticAnalyzer.enterVar(id, addr, false, table);
                } else {
                    semanticAnalyzer.enterVar(id, addr, table);
                }

                nextToken();
            } else {
                syntaxError(21); //21, "Missing argument name when declare PROCEDURE."
            }
        } while (token == SEMICOLON);

        if (token == RPARENT) {
            nextToken();
        } else {
            syntaxError(1); // 1, "Missing )"
        }
    }

    if (nameProc != nullptr) {
        semanticAnalyzer.setParameterProc(tblptr.back(), nameProc,
                                          argc, argv, codeGenerator.getCodeSize());
    }

    if (token == SEMICOLON) {
        nextToken();
        block();
        codeGenerator.gen(OP_EP);
        codeGenerator.setP(pos, codeGenerator.getCodeSize());
    } else {
        syntaxError(13); // 13, "Missing semicolon \';\'"
    }

    loadOldTable();
}

void Parser::term(void)
{
    factor();
    int op;
    while (token == TIMES || token == SLASH || token == PERCENT) {
        op = token;
        nextToken();
        factor();
        if (op == TIMES) {
            codeGenerator.gen(OP_MUL);
        } else if (op == SLASH) {
            codeGenerator.gen(OP_DIV);
        } else {
            codeGenerator.gen(OP_MOD);
        }
    }
}

void Parser::block(void)
{
    const static set<TokenType> FIRST_STATEMENT = {IDENT, BEGIN, CALL, IF, WHILE, FOR};
    if (token == CONST) {
        do {
            nextToken();
            declareConst();
        } while (token == COMMA);

        if (token == SEMICOLON) {
            nextToken();
        } else {
            syntaxError(13);// 13, "Missing semicolon \';\'"
        }
    }

    if (token == VAR) {
        do {
            nextToken();
            declareVariable();
        } while (token == COMMA);

        if (token == SEMICOLON) {
            nextToken();
        } else {
            syntaxError(13);// 13, "Missing semicolon \';\'"
        }
    }
    
    while (token == PROCEDURE) {
        nextToken();
        declareProc();

        if (token == SEMICOLON) {
            nextToken();
        } else {
            syntaxError(13);// 13, "Missing semicolon \';\'"
        }
    }
    
    if (table->width > 0) {
        codeGenerator.gen(OP_INT, 4 + table->width);
    }

    if (token == BEGIN) {
        nextToken();
    } else {
        syntaxError(15);// 15, "Missing keyword BEGIN"
    }

    statement();
    while (token == SEMICOLON || FIRST_STATEMENT.find(token) != FIRST_STATEMENT.end()) {
        if (token != SEMICOLON) {
            syntaxError(13);// 13, "Missing semicolon \';\'"
        } else {
            nextToken();
        }

        statement();
    };

    if (token == END) {
        nextToken();
    } else {
        syntaxError(5);// 5, "Missing keyword END"
    }
}

void Parser::checkRValue()
{
    unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
    id.reset(lexicalAnalyzer.getIdent());
    Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());
    if (entry != nullptr) {
        if (entry->kind == Object::PROCEDURE) {
            semanticError(29);// 29, "Procedure cannot be in Expression."
        }
    } else {
        semanticError(26, *id.get());// 26, "\' was not declared in this scope"
    }

    nextToken();
    if (token == LBRACK) {
        if (entry->kind == Object::VARIABLE || entry->kind == Object::CONST) {
            semanticError(33);// 33, "VARIABLE or CONST cannot be referenced by index array"
        }
        Addr addr = ((ArrayEntry*)entry)->addr;
        codeGenerator.gen(OP_LA, base - addr.base, addr.offset);
        nextToken();
        expression();
        codeGenerator.gen(OP_ADD);
        codeGenerator.gen(OP_LI);

        if (token == RBRACK) {
            nextToken();
        } else {
            syntaxError(0); // 0, "Missing ]"
        }
    } else {
        if (entry->kind == Object::ARRAY) {
            semanticError(35); // 35, "Missing array index"
        } else if (entry->kind == Object::VARIABLE) {
            codeGenerator.rvalue((VarEntry*)entry, base);
        } else if (entry->kind == Object::CONST) {
            codeGenerator.gen(OP_LC, ((ConstEntry*)entry)->val);
        }
    }
}

void Parser::factor(void)
{
    if (token == IDENT) {
        checkRValue();
    } else if (token == NUMBER) {
        codeGenerator.gen(OP_LC, (int)lexicalAnalyzer.getNumber());
        nextToken();
    } else if (token == LPARENT) {
        nextToken();
        expression();
        if (token == RPARENT) {
            nextToken();
        } else {
            syntaxError(1); // 1, "Missing )"
        }
    } else {
        syntaxError(4); // 4, "Missing factor."
    }
}

void Parser::expression()
{
    TokenType op = token;
    if (token == PLUS || token == MINUS) {
        nextToken();
    }
    term();
    if (op == MINUS) {
        codeGenerator.gen(OP_NEG);
    }
    while (token == PLUS || token == MINUS) {
        op = token;
        nextToken();
        term();
        if (op == PLUS) {
            codeGenerator.gen(OP_ADD);
        } else {
            codeGenerator.gen(OP_SUB);
        }
    }
}

void Parser::condition()
{
    const static set<TokenType> type = {EQU, NEQ, LSS, LEQ, GTR, GEQ};
    static unordered_map<int, OpCode> tokenToOpCode = {{EQU, OP_EQ}, {NEQ, OP_NE},
        {LSS, OP_LT}, {LEQ, OP_LE}, {GTR, OP_GT}, {GEQ, OP_GE}
    };

    if (token == ODD) {
        nextToken();
        expression();
        codeGenerator.gen(OP_ODD);
    } else {
        expression();
        TokenType op = token;
        if (type.find(token) != type.end()) {
            nextToken();
        } else  {
            syntaxError(2); // 2, "Condition: missing logic operator"
        }

        expression();
        codeGenerator.gen(tokenToOpCode[op]);
    }
}

void Parser::callProc()
{
    unique_ptr<string> nameProc;
    nameProc.reset(nullptr);
    ProcEntry *entry = nullptr;
    int pos = 0;
    if (token == IDENT) {
        nameProc.reset(lexicalAnalyzer.getIdent());
        entry = (ProcEntry*)semanticAnalyzer.getEntry(table, nameProc.get());

        if (entry != nullptr) {
            if (entry->kind != Object::PROCEDURE) {
                semanticError(37,*nameProc.get());//37, "\' is not a PROCEDURE"
            }
        } else {
            if (builtin.isExist(nameProc.get())) {
                semanticAnalyzer.enterProc(new string(*nameProc.get()), base, table);
                ProcInfo info = builtin.get(nameProc.get());

                pos = codeGenerator.getCodeSize();
                codeGenerator.gen(OP_J, 0);
                int address = codeGenerator.getCodeSize();
                info.p(codeGenerator);
                codeGenerator.setP(pos, codeGenerator.getCodeSize());
                semanticAnalyzer.setParameterProc(table, nameProc.get(), info.entry.argc, info.entry.argv, address);
                entry = (ProcEntry*)semanticAnalyzer.getEntry(table, nameProc.get());
            } else {
                semanticError(26, *nameProc.get());//26, "\' was not declared in this scope"
            }
        }
        nextToken();
    } else {
        syntaxError(23); // // 23, "Missing procedure name when call PROCEDURE."
    }
    int argc = 0;
    codeGenerator.gen(OP_INT, 4);

    if (token == LPARENT) {
        do {
            nextToken();
            if (entry != nullptr && entry->argv[argc]) {
                if (token == IDENT) {
                    unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
                    id.reset(lexicalAnalyzer.getIdent());
                    Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());
                    if (entry != nullptr) {
                        if (entry->kind != Object::VARIABLE && entry->kind != Object::ARRAY) {
                            semanticError(36); // 36, "Tham bien khong the la hang so, thu tuc hoac bi de trong"
                        }
                    } else {
                        semanticError(26, *id.get());// 26, "\' was not declared in this scope"
                    }

                    nextToken();
                    if (token == LBRACK) {
                        if (entry->kind == Object::VARIABLE || entry->kind == Object::CONST) {
                            semanticError(33); // 33, "VARIABLE or CONST cannot be referenced by index array"
                        }
                        Addr addr = ((ArrayEntry*)entry)->addr;
                        codeGenerator.gen(OP_LA, base - addr.base, addr.offset);
                        nextToken();
                        expression();
                        codeGenerator.gen(OP_ADD);

                        if (token == RBRACK) {
                            nextToken();
                        } else {
                            syntaxError(0); // 0, "Missing ]"
                        }
                    } else {
                        if (entry->kind == Object::ARRAY) {
                            semanticError(35); // 35, "Missing array index"
                        } else if (entry->kind == Object::VARIABLE) {
                            codeGenerator.lvalue((VarEntry*)entry, base); 
                        }
                    }
                } else {
                    semanticError(39); // 39, "Tham bien khong the bi de trong."
                }
            } else {
                expression();
            }

            argc++;
        } while (token == COMMA);

        if (token == RPARENT)
            nextToken();
        else
            syntaxError(1); // 1, "Missing )"
    }

    int result = 0;

    if (entry != nullptr) {
        codeGenerator.gen(OP_DCT, argc + 4);
        codeGenerator.gen(OP_CALL, base - entry->base, entry->address);
        result = semanticAnalyzer.checkParameterProc(table, nameProc.get(), argc);
    }

    if (result < 0) {
        semanticError(31, *nameProc.get());// 31, "Missing argument of procedure: "
    } else if (result > 0) {
        semanticError(32, *nameProc.get());// 32, "Too many argument of procedure: "
    }
}

void Parser::assignVariable()
{
    unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
    id.reset(lexicalAnalyzer.getIdent());
    Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());

    if (entry == nullptr) {
        semanticError(26, *id.get());// 26, "\' was not declared in this scope"
    } else {
        if (entry->kind != Object::VARIABLE && entry->kind != Object::ARRAY) {
            if (entry->kind == Object::PROCEDURE) {
                semanticError(28);// 28, "PROCEDURE can not be assigned."
            } else if (entry->kind == Object::CONST) {
                semanticError(27);// 27, "CONST can not be assigned."
            }
        }
    }

    nextToken();
    if (token == LBRACK) {
        if (entry != nullptr && entry->kind == Object::VARIABLE) {
            semanticError(33);// 33, "VARIABLE or CONST cannot be referenced by index array"
        }
        Addr addr = ((ArrayEntry*)entry)->addr;
        codeGenerator.gen(OP_LA, base - addr.base, addr.offset);
        nextToken();
        expression();
        codeGenerator.gen(OP_ADD);
        if (token == RBRACK) {
            nextToken();
        } else {
            syntaxError(0); // 0, "Missing ]"
        }
    } else {
        if (entry != nullptr) {
            if (entry->kind == Object::ARRAY) {
                semanticError(30);// 30, "Const ARRAY variable can not be assigned."
            } else {
                codeGenerator.lvalue((VarEntry*)entry, base); 
            }
        }
    }

    if (token == ASSIGN) {
        nextToken();
    } else {
        syntaxError(3); // 3, "Missing operator assign \':=\'"
    }
    expression();
    codeGenerator.gen(OP_ST);
}

void Parser::statement()
{
    const static set<TokenType> FIRST_STATEMENT = {IDENT, BEGIN, CALL, IF, WHILE, FOR};
    int pos1 = 0, pos2 = 0;

    if (token == IDENT) {
        assignVariable();
    } else if (token == CALL) {
        nextToken();
        callProc();
    } else if (token == BEGIN) {
        nextToken();
        statement();
        while (token == SEMICOLON || FIRST_STATEMENT.find(token) != FIRST_STATEMENT.end()) {
            if (token != SEMICOLON) {
                syntaxError(13); // 13, "Missing semicolon \';\'"
            } else {
                nextToken();
            }

            statement();
        };

        if (token == END) {
            nextToken();
        } else {
            syntaxError(5); // 5, "Missing keyword END"
        }
    } else if (token == IF) {
        nextToken();
        condition();
        pos1 = codeGenerator.getCodeSize();
        codeGenerator.gen(OP_FJ, 0);

        if (token == THEN) {
            nextToken();
        } else {
            syntaxError(6); // 6, "Missing keyword THEN"
        }

        statement();
        codeGenerator.setP(pos1, codeGenerator.getCodeSize());

        if (token == ELSE) {
            pos2 = codeGenerator.getCodeSize();
            codeGenerator.gen(OP_J, 0);
            codeGenerator.setP(pos1, codeGenerator.getCodeSize());
            nextToken();
            statement();
            codeGenerator.setP(pos2, codeGenerator.getCodeSize());
        }
    } else if (token == WHILE) {
        pos1 = codeGenerator.getCodeSize();
        nextToken();
        condition();
        if (token == DO) {
            pos2 = codeGenerator.getCodeSize();
            codeGenerator.gen(OP_FJ, 0);
            nextToken();
        } else {
            syntaxError(7); // 7, "Missing keyword DO in while loop"
        }
        statement();
        codeGenerator.gen(OP_J, pos1);
        codeGenerator.setP(pos2, codeGenerator.getCodeSize());
    } else if (token == FOR) {
        nextToken();
        if (token == IDENT) {
            unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
            id.reset(lexicalAnalyzer.getIdent());
            Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());
            if (entry == nullptr) {
                semanticError(26, *id.get());//26, "\' was not declared in this scope"
            } else {
                if (entry->kind != Object::VARIABLE) {
                    semanticError(38);//38, "IDENT in FOR loop must be a VARIABLE."
                }
            }

            Addr addr = ((VarEntry*)entry)->addr;
            if (((VarEntry*)entry)->PassByValue) {
                codeGenerator.gen(OP_LA, base - addr.base, addr.offset);
            } else {
                codeGenerator.gen(OP_LV, base - addr.base, addr.offset);
            }

            codeGenerator.gen(OP_CV);
            nextToken();
        } else {
            syntaxError(8); // 8, "Missing IDENT after FOR"
        }

        if (token == ASSIGN) {
            nextToken();
            expression();
            codeGenerator.gen(OP_ST);
            pos1 = codeGenerator.getCodeSize();
            codeGenerator.gen(OP_CV);
            codeGenerator.gen(OP_LI);

            if (token == TO) {
                nextToken();
            } else {
                syntaxError(9); // 9, "Missing keyword TO in for loop"
            }

            expression();
            codeGenerator.gen(OP_LE);
            pos2 = codeGenerator.getCodeSize();
            codeGenerator.gen(OP_FJ, 0);

            if (token == DO) {
                nextToken();
            } else {
                syntaxError(10); // 10, "Missing keyword DO in for loop"
            }

            statement();
            codeGenerator.gen(OP_CV);
            codeGenerator.gen(OP_CV);
            codeGenerator.gen(OP_LI);
            codeGenerator.gen(OP_LC, 1);
            codeGenerator.gen(OP_ADD);
            codeGenerator.gen(OP_ST);
            codeGenerator.gen(OP_J, pos1);
            codeGenerator.setP(pos2, codeGenerator.getCodeSize());
            codeGenerator.gen(OP_DCT, 1);
        } else {
            syntaxError(3); // 3, "Missing operator assign \':=\'"
        }
    }
}

void Parser::program(void)
{
    nextToken();
    if (token == PROGRAM) {
        nextToken();

        if (token == IDENT) {
            nextToken();
        } else {
            syntaxError(12); // 12, "Missing name of program"
        }

        if (token == SEMICOLON) {
            nextToken();
        } else {
            syntaxError(13); // 13, "Missing semicolon \';\'"
        }

        table = semanticAnalyzer.mktable(nullptr);
        block();
        codeGenerator.gen(OP_HL);
        if (token != PERIOD) {
            syntaxError(14); // 14, "Missing DOT \'.\'"
        }

        if (flag.numError == 0) {
            cout << endl << "Compile is SUCCESS" << endl;
            string output(this->name);
            output.append(".out");
            codeGenerator.save(output.c_str());
        } else {
            cout << endl << "Compile is NOT SUCCESS" << endl;
        }
        delete table;
    } else {
        syntaxError(11); // 11, "Missing keyword PROGRAM"
    }
}

int Parser::checkFileName(char* fileName)
{
    int i = strlen(fileName) - 1;

    while (i > -1) {
        if (fileName[i] == '.') {
            break;
        }
        i--;
    }

    if (i > 0) {
        if (!strcmp(&fileName[i + 1], "pl0")) {
            name.assign(fileName, i);
            return 0;
        }
    }

    cerr << fileName << " is not PL0 file." << endl;
    return -1;
}

int Parser::parse(char* fileName)
{
    if (checkFileName(fileName)) {
        return -1;
    }
    string input(this->name);
    input.append(".pl0");
    FILE *f = NULL;

    f = fopen(input.c_str(), "rt");
    if (f == NULL) {
        cout << "File is not exists." << endl;
        return -1;
    }
    /*
    unordered_map<int, const char*> ma = {
        {0, "NONE"}, {1, "IDENT"}, {2, "NUMBER"},
        {3, "BEGIN"}, {4, "CALL"}, {5, "CONST"}, {6, "DO"},  {7, "ELSE"}, {8, "END"}, {9, "FOR"}, {10, "IF"}, {11, "ODD"},
        {12, "PROCEDURE"}, {13, "PROGRAM"}, {14, "THEN"}, {15, "TO"}, {16, "VAR"}, {17, "WHILE"}, {18, "PLUS"}, {19, "MINUS"},
        {20, "TIMES"}, {21, "SLASH"}, {22, "EQU"}, {23, "NEQ"}, {24, "LSS"}, {25, "LEQ"}, {26, "GTR"}, {27, "GEQ"}, {28, "LPARENT"},
        {29, "RPARENT"}, {30, "LBRACK"}, {31, "RBRACK"}, {32, "PERIOD"}, {33, "COMMA"}, {34, "SEMICOLON"},  {35, "ASSIGN"}, {36, "PERCENT"}
    };
    */

    lexicalAnalyzer.init(f);
    
    program();

    fclose(f);
    if (flag.numError > 0) {
        return -1;
    }
    
    return 0;
}
