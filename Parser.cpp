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
    if(flag.printError) {
        flag.numError++;
        flag.printError = true; //Can sua lai thanh false

        cerr << "Line " << lexicalAnalyzer.getLine() << ": ";
        syntaxAnalyzer.error(errorCode, name);
    }
}

void Parser::semanticError(int errorCode, string name)
{
    if(flag.printError) {
        flag.numError++;
        flag.printError = true; //Can sua lai thanh false
        cerr << "Line " << lexicalAnalyzer.getLine() << ": ";
        semanticAnalyzer.error(errorCode, name);
    }
}


void Parser::nextToken(void)
{
    if(lexicalAnalyzer.hasNextToken()) {
        token = lexicalAnalyzer.getNextToken();
    } else {
        token = NONE;
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
    if(token == IDENT) {
        // Kiem tra khai bao bien
        string* id = lexicalAnalyzer.getIdent();
        if(semanticAnalyzer.checkIdent(table, id)) {
            semanticError(25, *id);
        }
        Addr addr;
        addr.base = base;
        addr.offset = offset;
        nextToken();
        if(token == LBRACK) {
            nextToken();
            if(token == NUMBER) {
                if(flag.printError) {
                    int len = (int)lexicalAnalyzer.getNumber();
                    semanticAnalyzer.enterArray(id, addr, table, len);
                    offset += len;
                }
                nextToken();
            } else {
                syntaxError(19);
            }
            if(token == RBRACK) {
                nextToken();
            } else {
                syntaxError(0);
            }
        } else {
            if(flag.printError) {
                semanticAnalyzer.enterVar(id, addr, table);
                offset++;
            }
        }
    } else {
        syntaxError(18);
    }
}

void Parser::declareConst()
{
    if(token == IDENT) {
        // Kiem tra khai bao bien
        string* id = lexicalAnalyzer.getIdent();
        if(semanticAnalyzer.checkIdent(table, id)) {
            semanticError(25, *id);
        }

        nextToken();
        if(token == EQU) {
            nextToken();
            if(token == NUMBER) {
                if(flag.printError) {
                    semanticAnalyzer.enterConst(id, table, lexicalAnalyzer.getNumber());
                }
                nextToken();
            } else {
                syntaxError(17);
            }
        } else {
            syntaxError(16);
        }
    } else {
        syntaxError(22);
    }
}

void Parser::declareProc()
{
    if(token == IDENT) {
        // Kiem tra khai bao procedure
        string* id = lexicalAnalyzer.getIdent();
        string* nameProc = id;
        int pos = 0;

        if(builtin.isExist(id)) {
            semanticError(34, *id);
        } else if(semanticAnalyzer.checkIdent(table, id)) {
            semanticError(25, *id);
        } else {
            tblptr.push_back(table);
            pos = codeGenerator.getCodeSize();
            codeGenerator.gen(OP_J, 0);
            table = semanticAnalyzer.enterProc(id, base, table);
            base++;
            offset = 4;
        }

        int argc = 0;
        vector<bool> argv;
        nextToken();
        if(token == LPARENT) {
            do {
                nextToken();
                argv.push_back(false);
                if(token == VAR) {
                    argv.back() = true;
                    nextToken();
                }
                if(token == IDENT) {
                    Addr addr;
                    addr.base = base;
                    addr.offset = offset++;
                    argc++;
                    id = lexicalAnalyzer.getIdent();
                    if(argv.back()) {
                        semanticAnalyzer.enterVar(id, addr, false, table);
                    } else {
                        semanticAnalyzer.enterVar(id, addr, table);
                    }

                    nextToken();
                } else {
                    syntaxError(21);
                }
            } while(token == SEMICOLON);

            if(token == RPARENT) {
                nextToken();
            } else {
                syntaxError(1);
            }
        }
        semanticAnalyzer.setParameterProc(table, nameProc, argc, argv, codeGenerator.getCodeSize());
        if(token == SEMICOLON) {
            nextToken();
            block();
            codeGenerator.gen(OP_EP);
            codeGenerator.setP(pos, codeGenerator.getCodeSize());
        } else {
            syntaxError(13);
        }

        loadOldTable();
    } else {
        syntaxError(20);
    }
}

void Parser::term(void)
{
    factor();
    int op;
    while(token == TIMES || token == SLASH || token == PERCENT) {
        op = token;
        nextToken();
        factor();
        if(op == TIMES) {
            codeGenerator.gen(OP_MUL);
        } else if(op == SLASH) {
            codeGenerator.gen(OP_DIV);
        } else {
            codeGenerator.gen(OP_MOD);
        }
    }
}

void Parser::block(void)
{
    while(token == CONST || token == VAR || token == PROCEDURE) {
        if(token == CONST) {
            do {
                nextToken();
                declareConst();
            } while(token == COMMA);
        } else if(token == VAR) {
            do {
                nextToken();
                declareVariable();
            } while(token == COMMA);
        } else {
            nextToken();
            declareProc();
        }

        if(token == SEMICOLON) {
            nextToken();
        } else {
            syntaxError(13);
        }
    }
    if(table->width > 0) {
        codeGenerator.gen(OP_INT, 4 + table->width);
    }

    if(token == BEGIN) {
        do {
            nextToken();
            statement();
        } while(token == SEMICOLON);

        if(token == END) {
            nextToken();
        } else {
            syntaxError(5);
        }
    } else {
        syntaxError(15);
    }
}

void Parser::checkFactor()
{
    unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
    id.reset(lexicalAnalyzer.getIdent());
    Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());
    if(entry != nullptr) {
        if(entry->kind == Object::PROCEDURE) {
            semanticError(29);// Thu tuc khong the o trong expression
        }
    } else {
        semanticError(26, *id.get());//Ten chua khai bao
    }

    nextToken();
    if(token == LBRACK) {
        if(entry->kind == Object::VARIABLE) {
            semanticError(33);
        }
        codeGenerator.gen(OP_LA);
        nextToken();
        expression();
        codeGenerator.gen(OP_ADD);
        codeGenerator.gen(OP_LI);

        if(token == RBRACK) {
            nextToken();
        } else {
            syntaxError(0);
        }
    } else {
        if(entry->kind == Object::ARRAY) {
            semanticError(35);
        } else if(entry->kind == Object::VARIABLE) {
            codeGenerator.lvalue((VarEntry*)entry, base);
        } else if(entry->kind == Object::CONST) {
            codeGenerator.gen(OP_LC, ((ConstEntry*)entry)->val);
        }
    }
}

void Parser::factor(void)
{
    if(token == IDENT) {
        checkFactor();
    } else if(token == NUMBER) {
        codeGenerator.gen(OP_LC, (int)lexicalAnalyzer.getNumber());
        nextToken();
    } else if(token == LPARENT) {
        nextToken();
        expression();
        if(token == RPARENT) {
            nextToken();
        } else {
            syntaxError(1);
        }
    } else {
        syntaxError(24);
    }
}

void Parser::expression()
{
    TokenType op = token;
    if(token == PLUS || token == MINUS) {
        nextToken();
    }
    term();
    if(op == MINUS) {
        codeGenerator.gen(OP_NEG);
    }
    while(token == PLUS || token == MINUS) {
        op = token;
        nextToken();
        term();
        if(op == PLUS) {
            codeGenerator.gen(OP_ADD);
        } else {
            codeGenerator.gen(OP_SUB);
        }
    }
}

void Parser::condition()
{
    const static set<TokenType> type = {EQU, NEQ, LSS, LEQ, GTR, GEQ};
    static unordered_map<TokenType, OpCode> tokenToOpCode = {{EQU, OP_EQ}, {NEQ, OP_NE},
        {LSS, OP_LT}, {LEQ, OP_LE}, {GTR, OP_GT}, {GEQ, OP_GE}
    };

    if(token == ODD) {
        nextToken();
        expression();
        codeGenerator.gen(OP_ODD);
    } else {
        expression();
        TokenType op = token;
        if(type.find(token) != type.end()) {
            nextToken();
            expression();
            codeGenerator.gen(tokenToOpCode[op]);
        } else	{
            syntaxError(3);
        }
    }
}

void Parser::callProc()
{
    if(token == IDENT) {
        unique_ptr<string> nameProc;
        nameProc.reset(lexicalAnalyzer.getIdent());

        ProcEntry *entry = (ProcEntry*)semanticAnalyzer.getEntry(table, nameProc.get());
        int pos = 0;
        if(entry != nullptr) {
            if(entry->kind != Object::PROCEDURE) {
                semanticError(28);//Ve trai phai la mot procedure
            }
        } else {
            if(builtin.isExist(nameProc.get())) {
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
                semanticError(26, *nameProc.get());//Ten chua khai bao
            }
        }

        int argc = 0;
        nextToken();
        codeGenerator.gen(OP_INT, 4);
        if(token == LPARENT) {
            do {
                nextToken();
                if(entry->argv[argc]) {
                    if(token == IDENT) {
                        unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
                        id.reset(lexicalAnalyzer.getIdent());
                        Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());
                        if(entry != nullptr) {
                            if(entry->kind == Object::PROCEDURE || entry->kind == Object::CONST) {
                                semanticError(36);
                            }
                        } else {
                            semanticError(26, *id.get());//Ten chua khai bao
                        }

                        nextToken();
                        if(token == LBRACK) {
                            if(entry->kind == Object::VARIABLE) {
                                semanticError(33);
                            }
                            codeGenerator.gen(OP_LA);
                            nextToken();
                            expression();
                            codeGenerator.gen(OP_ADD);

                            if(token == RBRACK) {
                                nextToken();
                            } else {
                                syntaxError(0);
                            }
                        } else {
                            if(entry->kind == Object::ARRAY) {
                                semanticError(35);
                            } else if(entry->kind == Object::VARIABLE) {
                                Addr addr = ((VarEntry*)entry)->addr;
                                codeGenerator.gen(OP_LA, base-addr.base, addr.offset);
                            }
                        }
                    } else {
                        semanticError(36);
                    }
                } else {
                    expression();
                }

                argc++;
            } while(token == COMMA);

            if(token == RPARENT)
                nextToken();
            else
                syntaxError(1);
        }

        codeGenerator.gen(OP_DCT, argc + 4);
        codeGenerator.gen(OP_CALL, base - entry->base, entry->address);

        int result = semanticAnalyzer.checkParameterProc(table, nameProc.get(), argc);
        if(result < 0) {
            semanticError(31, *nameProc.get());
        } else if(result > 0) {
            semanticError(32, *nameProc.get());
        }
    } else {
        syntaxError(23);
    }
}

void Parser::assignVariable()
{
    unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
    id.reset(lexicalAnalyzer.getIdent());
    Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());

    if(entry == nullptr) {
        semanticError(26, *id.get());//Ten chua khai bao
    } else {
        if(entry->kind != Object::VARIABLE && entry->kind != Object::ARRAY) {
            semanticError(27);//Ve trai phai la mot bien
        }
    }

    nextToken();
    if(token == LBRACK) {
        if(entry != nullptr && entry->kind == Object::VARIABLE) {
            semanticError(33);
        }
        Addr addr = ((ArrayEntry*)entry)->addr;
        codeGenerator.gen(OP_LA, base-addr.base, addr.offset);
        nextToken();
        expression();
        codeGenerator.gen(OP_ADD);
        if(token == RBRACK) {
            nextToken();
        } else {
            syntaxError(0);
        }
    } else {
        if(entry != nullptr) {
            if(entry->kind == Object::ARRAY) {
                semanticError(30);
            } else {
                Addr addr = ((VarEntry*)entry)->addr;
                if(((VarEntry*)entry)->PassByValue) {
                    codeGenerator.gen(OP_LA, base - addr.base, addr.offset);
                } else {
                    codeGenerator.gen(OP_LV, base - addr.base, addr.offset);
                }
            }
        }
    }

    if(token==ASSIGN) {
        nextToken();
        expression();
    } else {
        syntaxError(4);
    }

    codeGenerator.gen(OP_ST);
}

void Parser::statement()
{
    int pos1 = 0, pos2 = 0;
    if(token == IDENT) {
        assignVariable();
    } else if (token == CALL) {
        nextToken();
        callProc();
    } else if (token == BEGIN) {
        do {
            nextToken();
            statement();
        } while(token == SEMICOLON);

        if(token == END) {
            nextToken();
        } else {
            syntaxError(5);
        }
    } else if (token == IF) {
        nextToken();
        condition();
        pos1 = codeGenerator.getCodeSize();
        codeGenerator.gen(OP_FJ, 0);
        if(token == THEN) {
            nextToken();
            statement();
            codeGenerator.setP(pos1, codeGenerator.getCodeSize());
            if(token == ELSE) {
                pos2 = codeGenerator.getCodeSize();
                codeGenerator.gen(OP_J, 0);
                codeGenerator.setP(pos1, codeGenerator.getCodeSize());
                nextToken();
                statement();
                codeGenerator.setP(pos2, codeGenerator.getCodeSize());
            }
        } else {
            syntaxError(6);
        }
    } else if (token == WHILE) {
        pos1 = codeGenerator.getCodeSize();
        nextToken();
        condition();
        if(token == DO) {
            pos2 = codeGenerator.getCodeSize();
            codeGenerator.gen(OP_FJ, 0);
            nextToken();
            statement();
            codeGenerator.gen(OP_J, pos1);
            codeGenerator.setP(pos2, codeGenerator.getCodeSize());
        } else {
            syntaxError(7);
        }
    } else if (token == FOR) {
        nextToken();
        if(token == IDENT) {
            unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
            id.reset(lexicalAnalyzer.getIdent());
            Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());
            if(entry == nullptr) {
                semanticError(26, *id.get());//Ten chua khai bao
            } else {
                if(entry->kind != Object::VARIABLE) {
                    semanticError(27);//Ve trai phai la mot bien
                }
            }

            Addr addr = ((VarEntry*)entry)->addr;
            if(((VarEntry*)entry)->PassByValue) {
                codeGenerator.gen(OP_LA, base - addr.base, addr.offset);
            } else {
                codeGenerator.gen(OP_LV, base - addr.base, addr.offset);
            }

            codeGenerator.gen(OP_CV);
            nextToken();
            if(token == ASSIGN) {
                nextToken();
                expression();
                codeGenerator.gen(OP_ST);
                pos1 = codeGenerator.getCodeSize();
                codeGenerator.gen(OP_CV);
                codeGenerator.gen(OP_LI);
                if(token == TO) {
                    nextToken();
                    expression();
                    codeGenerator.gen(OP_LE);
                    pos2 = codeGenerator.getCodeSize();
                    codeGenerator.gen(OP_FJ, 0);
                    if(token == DO) {
                        nextToken();
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
                        syntaxError(10);
                    }
                } else {
                    syntaxError(9);
                }
            } else {
                syntaxError(3);
            }
        } else {
            syntaxError(8);
        }
    }
}

void Parser::program(void)
{
    nextToken();
    if(token == PROGRAM) {
        nextToken();
        if (token == IDENT) {
            nextToken();
            if(token == SEMICOLON) {
                nextToken();
                table = semanticAnalyzer.mktable(nullptr);
                block();
                codeGenerator.gen(OP_HL);
                if(token == PERIOD) {
                    if(flag.numError == 0) {
                        cout << "SUCCESS" << endl;
                        string output(this->name);
                        output.append(".out");
                        codeGenerator.save(output.c_str());
                    }
                } else {
                    syntaxError(14);
                }
                delete table;
            } else {
                syntaxError(13);
            }
        } else {
            syntaxError(12);
        }
    } else {
        syntaxError(11);
    }
}

int Parser::checkFileName(char* fileName)
{
    int i = strlen(fileName) - 1;

    while(i > -1) {
        if(fileName[i] == '.') {
            break;
        }
        i--;
    }

    if(i > 0) {
        if(!strcmp(&fileName[i + 1], "pl0")) {
            name.assign(fileName, i);
            return 0;
        }
    }

    cerr << fileName << " is not PL0 file." << endl;
    return -1;
}

void Parser::parse(char* fileName)
{
    if(checkFileName(fileName)) {
        return;
    }
    string input(this->name);
    input.append(".pl0");
    FILE *f = NULL;
    f = fopen(input.c_str(), "rt");
    if(f == NULL) {
        cout << "File is not exists." << endl;
        return;
    }
    unordered_map<int, const char*> ma = {
        {0, "NONE"}, {1, "IDENT"}, {2, "NUMBER"},
        {3, "BEGIN"}, {4, "CALL"}, {5, "CONST"}, {6, "DO"},  {7, "ELSE"}, {8, "END"}, {9, "FOR"}, {10, "IF"}, {11, "ODD"},
        {12, "PROCEDURE"}, {13, "PROGRAM"}, {14, "THEN"}, {15, "TO"}, {16, "VAR"}, {17, "WHILE"},{18, "PLUS"}, {19, "MINUS"},
        {20, "TIMES"}, {21, "SLASH"}, {22, "EQU"}, {23, "NEQ"}, {24, "LSS"}, {25, "LEQ"}, {26, "GTR"}, {27, "GEQ"}, {28, "LPARENT"},
        {29, "RPARENT"}, {30, "LBRACK"}, {31, "RBRACK"}, {32, "PERIOD"}, {33, "COMMA"}, {34, "SEMICOLON"},  {35, "ASSIGN"}, {36, "PERCENT"}
    };

    lexicalAnalyzer.setFile(f);
    /*
    while(lexicalAnalyzer.hasNextToken()) {
        cout << ma[lexicalAnalyzer.getNextToken()] << '|';
    }*/

    program();

    fclose(f);
}
