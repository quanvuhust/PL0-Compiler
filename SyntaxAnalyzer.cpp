#include "SyntaxAnalyzer.h"
#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include <iostream>
#include <set>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cassert>

using namespace std;

extern Flag flag;

void SyntaxAnalyzer::error(int errorCode, string name)
{
    if(flag.printError) {
        static unordered_map<int, const char*> errorString = {{0, "Missing ]"}, {1, "Missing )"},
            {2, "Condition: missing logic operator"}, {3, "Missing operator :="},
            {4, "Missing ("}, {5, "Missing keyword END"}, {6, "Missing keyword THEN"}, {7, "Missing keyword DO in while loop"},
            {8, "Missing IDENT after FOR"}, {9, "Missing keyword TO in for loop"},
            {10, "Missing keyword DO in for loop"}, {11, "Missing keyword PROGRAM"},
            {12, "Missing name program"}, {13, "Missing semicolon \';\'"}, {14, "Missing DOT \'.\'"},
            {15, "Missing block"}, {16, "Missing operator \'=\'"}, {17, "Missing number when assign a const"},
            {18, "Missing variable name."}, {19, "Missing array index."}, {20, "Missing procedure name."},
            {21, "Missing argument name when declare PROCEDURE."}, {22, "Missing const name."},
            {23, "Missing procedure name when call PROCEDURE."}, {24, "Missing factor."}, {25, "Redeclaration of "},
            {26, "\' was not declared in this scope"}, {27, "Must be a variable."}, {28, "lvalue is not procedure."},
            {29, "Procedure cannot be in Expression."}, {30, "Const array variable cannot assign."},
            {31, "Missing argument of procedure: "}, {32, "Too many argument of procedure: "},
            {33, "Variable cannot be referenced by index array"}
        };
        flag.numError++;
        flag.printError = true; //Can sua lai thanh false
        if(errorCode == 26) {
            cerr << "Line " << lexicalAnalyzer.getLine() << ": \'" << name << errorString[errorCode] << endl;
        } else {
            cerr << "Line " << lexicalAnalyzer.getLine() << ": " << errorString[errorCode] << name << endl;
        }
    }
}

void SyntaxAnalyzer::nextToken(void)
{
    if(lexicalAnalyzer.hasNextToken()) {
        token = lexicalAnalyzer.getNextToken();
    } else {
        token = NONE;
    }
}

void SyntaxAnalyzer::loadOldTable()
{
    table->removeAllEntry();
    delete table;
    table = tblptr.back();
    tblptr.pop_back();
}

void SyntaxAnalyzer::declareVariable()
{
    if(token == IDENT) {
        // Kiem tra khai bao bien
        string* id = lexicalAnalyzer.getIdent();
        if(semanticAnalyzer.checkIdent(table, id)) {
            error(25, *id);
        }

        nextToken();
        if(token == LBRACK) {
            nextToken();
            if(token == NUMBER) {
                if(flag.printError) {
                    semanticAnalyzer.enterArray(id, table, (int)lexicalAnalyzer.getNumber());
                }
                nextToken();
            } else {
                error(19);
            }
            if(token == RBRACK) {
                nextToken();
            } else {
                error(0);
            }
        } else {
            if(flag.printError) {
                semanticAnalyzer.enterVar(id, table);
            }
        }
    } else {
        error(18);
    }
}

void SyntaxAnalyzer::declareConst()
{
    if(token == IDENT) {
        // Kiem tra khai bao bien
        string* id = lexicalAnalyzer.getIdent();
        if(semanticAnalyzer.checkIdent(table, id)) {
            error(25, *id);
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
                error(17);
            }
        } else {
            error(16);
        }
    } else {
        error(22);
    }
}

void SyntaxAnalyzer::declareProc()
{
    if(token == IDENT) {
        // Kiem tra khai bao procedure
        string* id = lexicalAnalyzer.getIdent();
        string* nameProc = id;
        if(semanticAnalyzer.checkIdent(table, id)) {
            error(25, *id);
        } else {
            tblptr.push_back(table);
            table = semanticAnalyzer.enterProc(id, table);
        }
        int argc = 0;
        vector<string*> argv;
        nextToken();
        if(token == LPARENT) {
            do {
                nextToken();
                if(token == VAR) {
                    nextToken();
                }
                if(token == IDENT) {
                    argc++;
                    id = lexicalAnalyzer.getIdent();
                    argv.push_back(id);
                    semanticAnalyzer.enterVar(id, table);
                    nextToken();
                } else {
                    error(21);
                }
            } while(token == SEMICOLON);

            if(token == RPARENT) {
                nextToken();
            } else {
                error(1);
            }
        }
        semanticAnalyzer.setParameterProc(table, nameProc, argc, argv);
        if(token == SEMICOLON) {
            nextToken();
            block();
        } else {
            error(13);
        }

        loadOldTable();
    } else {
        error(20);
    }
}

void SyntaxAnalyzer::term(void)
{
    factor();
    while(token == TIMES || token == SLASH || token == PERCENT) {
        nextToken();
        factor();
    }
}

void SyntaxAnalyzer::block(void)
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
            error(13);
        }
    }

    if(token == BEGIN) {
        // Luu table cua pham vi cha vao Stack
        tblptr.push_back(table);
        table = semanticAnalyzer.mktable(table);
        do {
            nextToken();
            statement();
        } while(token == SEMICOLON);

        if(token == END) {
            nextToken();
        } else {
            error(5);
        }

        loadOldTable();
    } else {
        error(15);
    }
}

void SyntaxAnalyzer::checkFactor()
{
    unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
    id.reset(lexicalAnalyzer.getIdent());
    Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());
    if(entry != nullptr) {
        if(entry->kind == Object::PROCEDURE) {
            error(29);// Thu tuc khong the o trong expression
        }
    } else {
        error(26, *id.get());//Ten chua khai bao
    }
}

void SyntaxAnalyzer::factor(void)
{
    if(token == IDENT) {
        checkFactor();
        nextToken();
        if(token == LBRACK) {
            expression();
            if(token == RBRACK) {
                nextToken();
            } else {
                error(0);
            }
        }
    } else if(token == NUMBER) {
        nextToken();
    } else if(token == LPARENT) {
        nextToken();
        expression();
        if(token == RPARENT) {
            nextToken();
        } else {
            error(1);
        }
    } else {
        error(24);
    }
}

void SyntaxAnalyzer::expression()
{
    if(token == PLUS || token == MINUS)
        nextToken();
    term();
    while(token == PLUS || token == MINUS) {
        nextToken();
        term();
    }
}

void SyntaxAnalyzer::condition()
{
    const static set<TokenType> type = {EQU, NEQ, LSS, LEQ, GTR, GEQ};
    if(token == ODD) {
        nextToken();
        expression();
    } else {
        expression();
        if(type.find(token) != type.end()) {
            nextToken();
            expression();
        } else	{
            error(3);
        }
    }
}

void SyntaxAnalyzer::callProc()
{
    if(token == IDENT) {
        unique_ptr<string> nameProc;
        nameProc.reset(lexicalAnalyzer.getIdent());
        Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, nameProc.get());

        if(entry != nullptr) {
            if(entry->kind != Object::PROCEDURE) {
                error(28);//Ve trai phai la mot procedure
            }
        } else {
            error(26, *nameProc.get());//Ten chua khai bao
        }

        int argc = 0;
        nextToken();
        if(token == LPARENT) {
            do {
                nextToken();
                expression();
                argc++;
            } while(token == COMMA);

            if(token == RPARENT)
                nextToken();
            else
                error(1);
        }

        int result = semanticAnalyzer.checkParameterProc(table, nameProc.get(), argc);
        if(result < 0) {
            error(31, *nameProc.get());
        } else if(result > 0) {
            error(32, *nameProc.get());
        }
    } else {
        error(23);
    }
}

void SyntaxAnalyzer::assignVariable()
{
    unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
    id.reset(lexicalAnalyzer.getIdent());
    Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());

    if(entry == nullptr) {
        error(26, *id.get());//Ten chua khai bao
    } else {
        if(entry->kind != Object::VARIABLE && entry->kind != Object::ARRAY) {
            error(27);//Ve trai phai la mot bien
        }
    }

    nextToken();
    if(token == LBRACK) {
        if(entry != nullptr && entry->kind == Object::VARIABLE) {
            error(33);
        }
        nextToken();
        expression();
        if(token == RBRACK) {
            nextToken();
        } else {
            error(0);
        }
    } else {
        if(entry != nullptr && entry->kind == Object::ARRAY) {
            error(30);
        }
    }

    if(token==ASSIGN) {
        nextToken();
        expression();
    } else {
        error(4);
    }
}

void SyntaxAnalyzer::statement()
{
    if(token == IDENT) {
        // Kiem tra bien da duoc khai bao chua khi dung
        assignVariable();
    } else if (token == CALL) {
        nextToken();
        callProc();
    } else if (token == BEGIN) {
        // Luu table cua pham vi cha vao Stack
        tblptr.push_back(table);
        table = semanticAnalyzer.mktable(table);
        do {
            nextToken();
            statement();
        } while(token == SEMICOLON);

        if(token == END) {
            nextToken();
        } else {
            error(5);
        }
        // Khoi phuc lai table cua pham vi cha
        loadOldTable();
    } else if (token == IF) {
        nextToken();
        condition();
        if(token == THEN) {
            nextToken();
            statement();
            if(token == ELSE) {
                nextToken();
                statement();
            }
        } else {
            error(6);
        }
    } else if (token == WHILE) {
        nextToken();
        condition();
        if(token == DO) {
            nextToken();
            statement();
        } else {
            error(7);
        }
    } else if (token == FOR) {
        nextToken();
        if(token == IDENT) {
            unique_ptr<string> id; // Con tro thong minh tu dong xoa sau khi ra khoi block
            id.reset(lexicalAnalyzer.getIdent());
            Entry *entry = (Entry*)semanticAnalyzer.getEntry(table, id.get());
            if(entry == nullptr) {
                error(26, *id.get());//Ten chua khai bao
            } else {
                if(entry->kind != Object::VARIABLE) {
                    error(27);//Ve trai phai la mot bien
                }
            }

            nextToken();
            if(token == ASSIGN) {
                nextToken();
                expression();
                if(token == TO) {
                    nextToken();
                    expression();
                    if(token == DO) {
                        nextToken();
                        statement();
                    } else {
                        error(10);
                    }
                } else {
                    error(9);
                }
            } else {
                error(3);
            }
        } else {
            error(8);
        }
    }
}

void SyntaxAnalyzer::program(void)
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
                if(token == PERIOD) {
                    if(flag.numError == 0)
                        cout << "SUCCESS" << endl;
                } else {
                    error(14);
                }
                table->removeAllEntry();
                delete table;
            } else {
                error(13);
            }
        } else {
            error(12);
        }
    } else {
        error(11);
    }
}

