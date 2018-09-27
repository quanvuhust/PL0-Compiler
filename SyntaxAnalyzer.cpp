#include "SyntaxAnalyzer.h"
#include "LexicalAnalyzer.h"
#include <iostream>
#include <set>
#include <unordered_map>

using namespace std;

extern int flagError;

void SyntaxAnalyzer::error(int errorCode)
{
    static unordered_map<int, const char*> errorString = {{0, "Missing ]"}, {1, "Missing )"},
        {2, "Condition: missing logic operator"}, {3, "Missing operator :="},
        {4, "Missing ("}, {5, "Missing keyword END"}, {6, "Missing keyword THEN"}, {7, "Missing keyword DO in while loop"},
        {8, "Missing IDENT after FOR"}, {9, "Missing keyword TO in for loop"},
        {10, "Missing keyword DO in for loop"}, {11, "Missing keyword PROGRAM"},
        {12, "Missing name program"}, {13, "Missing semicolon \';\'"}, {14, "Missing DOT \'.\'"},
        {15, "Missing block"}, {16, "Missing operator \'=\'"}, {17, "Missing number when assign a const"},
        {18, "Missing variable name."}, {19, "Missing array index."}, {20, "Missing procedure name."},
        {21, "Missing argument name when declare PROCEDURE."}, {22, "Missing const name."},
        {23, "Missing procedure name when call PROCEDURE"}, {24, "Missing factor"}
    };
    flagError = 1;

    cerr << "Line " << lexicalAnalyzer.getLine() << ": " << errorString[errorCode] << endl;
}

void SyntaxAnalyzer::nextToken(void)
{
    if(lexicalAnalyzer.hasNextToken()) {
        token = lexicalAnalyzer.getNextToken();
    } else {
        token = NONE;
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
                if(token == IDENT) {
                    nextToken();
                    if(token == EQU) {
                        nextToken();
                        if(token != NUMBER) {
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
            } while(token == COMMA);
        } else if(token == VAR) {
            do {
                nextToken();
                if(token == IDENT) {
                    nextToken();
                    if(token == LBRACK) {
                        nextToken();
                        if(token == NUMBER) {
                            nextToken();
                        } else {
                            error(19);
                        }
                        if(token == RBRACK) {
                            nextToken();
                        } else {
                            error(0);
                        }
                    }
                } else {
                    error(18);
                }
            } while(token == COMMA);
        } else {
            nextToken();
            if(token == IDENT) {
                nextToken();
                if(token == LPARENT) {
                    do {
                        nextToken();
                        if(token == VAR) {
                            nextToken();
                        }
                        if(token == IDENT) {
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

                if(token == SEMICOLON) {
                    nextToken();
                    block();
                } else {
                    error(13);
                }
            } else {
                error(20);
            }
        }

        if(token == SEMICOLON) {
            nextToken();
        } else {
            error(13);
        }
    }

    if(token == BEGIN) {
        do{
            nextToken();
            statement();
        } while(token == SEMICOLON);

        if(token == END) {
            nextToken();
        } else {
            error(5);
        }
    } else {
        error(15);
    }
}

void SyntaxAnalyzer::factor(void)
{
    if(token == IDENT) {
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

void SyntaxAnalyzer::statement()
{
    if(token == IDENT) {
        nextToken();
        if(token == LBRACK) {
            nextToken();
            expression();
            if(token == RBRACK) {
                nextToken();
            } else {
                error(0);
            }
        }

        if(token==ASSIGN) {
            nextToken();
            expression();
        } else {
            error(4);
        }
    } else if (token == CALL) {
        nextToken();
        if(token == IDENT) {
            nextToken();
            if(token == LPARENT) {
                nextToken();
                expression();
                while(token == COMMA) {
                    nextToken();
                    expression();
                }
                if(token == RPARENT)
                    nextToken();
                else
                    error(1);
            }
        } else {
            error(23);
        }
    } else if (token == BEGIN) {
        nextToken();
        statement();
        while(token == SEMICOLON) {
            statement();
        }
        if(token == END) {
            nextToken();
        } else {
            error(5);
        }
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
                block();
                if(token == PERIOD) {
                    if(flagError == 0)
                        cout << "SUCCESS" << endl;
                }
                else
                    error(14);
            }
            else
                error(13);
        }
        else {
            error(12);
        }
    }
    else {
        error(11);
    }
}

