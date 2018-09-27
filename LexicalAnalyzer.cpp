#include "LexicalAnalyzer.h"
#include <iostream>
#include <cctype>
#include <unordered_map>
#include <string>
#include <algorithm>

using namespace std;
extern int flagError;

long LexicalAnalyzer::getLine(void)
{
    return line;
}

bool LexicalAnalyzer::hasNextToken()
{
    return !eofFlag;
}

void LexicalAnalyzer::reset(void)
{
    ide.clear();
    num.clear();
    state = 0;
}

bool LexicalAnalyzer::getCh(void)
{
    if(!noReadMore) {
        c = fgetc(f);
        if(c == '\n') {
            line++;
        }
        if(c == -1) {
            eofFlag = true;
            return 1;
        }
        return 0;
    } else {
        noReadMore = false;
    }
}

TokenType LexicalAnalyzer::getNextToken()
{
    reset();
    while(!getCh()) {
        next();
        if(this->state == ENDSTATE) {
            return this->token;
        }
    }
    return NONE;
}

unsigned char LexicalAnalyzer::getNextState0()
{
    const static unordered_map<char, TokenType> SINGLE_SYMBOL = {{'+', PLUS},
        {'-', MINUS}, {'*', TIMES}, {'/', SLASH}, {'%', PERCENT}, {'(', LPARENT},
        {')', RPARENT}, {'[', LBRACK}, {']', RBRACK}, {'=', EQU}, {',', COMMA},
        {';', SEMICOLON}, {'.', PERIOD}
    };
    const static unordered_map<char, int> DOUBLE_SYMBOL = {{':', 5},
        {'<', 4}, {'>', 3}
    };

    if(isalpha(c)) {
        ide.emplace_back(c);
        return 1;
    } else if(isdigit(c)) {
        num.emplace_back(c);
        return 2;
    } else if(isspace(c))
        return 0;
    else if(SINGLE_SYMBOL.find(c) != SINGLE_SYMBOL.end()) {
        this->token = SINGLE_SYMBOL.at(c);
        return ENDSTATE;
    } else if(DOUBLE_SYMBOL.find(c) != DOUBLE_SYMBOL.end()) {
        return DOUBLE_SYMBOL.at(c);
    } else {
        error("Invalid symbols.");
        this->token = NONE;
        return ENDSTATE;
    }

}

unsigned char LexicalAnalyzer::getNextState1()
{
    const static  unordered_map<string, TokenType> RESERVED_WORDS = {
        {"begin", BEGIN}, {"call", CALL}, {"end", END}, {"const", CONST}, {"do", DO},
        {"else", ELSE}, {"for", FOR}, {"if", IF}, {"odd", ODD}, {"procedure", PROCEDURE},
        {"program", PROGRAM}, {"then", THEN}, {"to", TO}, {"var", VAR}, {"while", WHILE}
    };

    if(isalpha(c) || isdigit(c)) {
        if(ide.size() <= MAX_IDENT_LEN + 1)
            ide.emplace_back(c);

        return 1;
    } else {
        if(ide.size() == MAX_IDENT_LEN + 1) {
            error("Error 1: Name too long.");
        }
        string lexeme(ide.begin(), ide.end());
        transform(lexeme.begin(), lexeme.end(), lexeme.begin(), ::tolower);
        if(RESERVED_WORDS.find(lexeme) != RESERVED_WORDS.end())
            this->token = RESERVED_WORDS.at(lexeme);
        else
            this->token = IDENT;
        noReadMore = true;
        return ENDSTATE;
    }
}

unsigned char LexicalAnalyzer::getNextState2()
{
    if(isdigit(c)) {
        if(num.size() <= MAX_NUMBER_LEN + 1)
            num.emplace_back(c);

        return 2;
    } else {
        if(num.size() == MAX_NUMBER_LEN + 1) {
            error("ERROR 2: Number too long.");
        }
        this->token = NUMBER;
        noReadMore = true;
        return ENDSTATE;
    }
}

unsigned char LexicalAnalyzer::getNextState3()
{
    if(c == '=')
        this->token = GEQ;
    else
        this->token = GTR;

    return ENDSTATE;
}

unsigned char LexicalAnalyzer::getNextState4()
{
    if(c == '>')
        this->token = NEQ;
    else if(c == '=')
        this->token = LEQ;
    else
        this->token = LSS;

    return ENDSTATE;
}

unsigned char LexicalAnalyzer::getNextState5()
{
    if(c == '=')
        this->token = ASSIGN;
    else {
        error("ERROR 3: Invalid symbols.");
        this->token = NONE;
    }

    return ENDSTATE;
}

void LexicalAnalyzer::error(const char* msg)
{
    flagError++;
    cerr << "Line " << getLine() << ": " << msg << endl;
}
