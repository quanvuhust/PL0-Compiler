#include "LexicalAnalyzer.h"
#include <iostream>
#include <cctype>
#include <unordered_map>
#include <algorithm>
#include <cstring>

using namespace std;
extern Flag flag;

void LexicalAnalyzer::init(FILE *f)
{
    this->f = f;
}

long LexicalAnalyzer::getLine(void)
{
    return line;
}

bool LexicalAnalyzer::hasNextToken()
{
    return this->token != NONE;
}

string* LexicalAnalyzer::getIdent() {
    return new string(ide.begin(), ide.end());
}

NumberType LexicalAnalyzer::getNumber() {
    return atoi(string(num.begin(), num.end()).c_str());
}

void LexicalAnalyzer::reset(void)
{
    ide.clear();
    num.clear();
    state = 0;
}

void LexicalAnalyzer::getCh(void)
{
    if(!noReadMore) {
        if(lenBuffer == first) {
            line++;
            first = 0;
            if(fgets(buffer, MAX_BUFFER, f) != NULL) {
                lenBuffer = strlen(buffer);
                if(feof(f)) {
                    buffer[lenBuffer++] = -1;
                }
            } else {
                buffer[0] = -1;
            }
        }
        c = buffer[first++];
    } else {
        noReadMore = false;
    }
}

TokenType LexicalAnalyzer::getNextToken()
{
    reset();
    while(1) {
        getCh();
        next();
        if(this->state == ENDSTATE) {
            return this->token;
        }
    }
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
        ide.emplace_back(tolower(c));
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
    } else if(c == -1) {
        this->token = NONE;
        return ENDSTATE;
    } else {
        error("Invalid symbols.");
        return 0;
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
        if(ide.size() < MAX_IDENT_LEN + 1) {
            ide.emplace_back(tolower(c));
        }

        return 1;
    } else {
        if(ide.size() == MAX_IDENT_LEN + 1) {
            error("Error 1: Name too long.");
            ide.pop_back();
        }
        string lexeme(ide.begin(), ide.end());
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
        if(num.size() < MAX_NUMBER_LEN + 1)
            num.emplace_back(c);

        return 2;
    } else {
        if(num.size() == MAX_NUMBER_LEN + 1) {
            error("ERROR 2: Number too long.");
            num.pop_back();
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
    else {
        this->token = GTR;
        noReadMore = true;
    }

    return ENDSTATE;
}

unsigned char LexicalAnalyzer::getNextState4()
{
    if(c == '>')
        this->token = NEQ;
    else if(c == '=')
        this->token = LEQ;
    else {
        this->token = LSS;
        noReadMore = true;
    }

    return ENDSTATE;
}

unsigned char LexicalAnalyzer::getNextState5()
{
    if(c == '=')
        this->token = ASSIGN;
    else {
        error("ERROR 3: Invalid symbols.");
        noReadMore = true;
        return 0;
    }

    return ENDSTATE;
}

void LexicalAnalyzer::error(const char* msg)
{
    flag.numError++;
    cerr << "Line " << getLine() << ": " << msg << endl;
}
