#ifndef LEXICAL_ANALYZER
#define LEXICAL_ANALYZER 0

#include <cstdio>
#include <vector>
#include <string>
#include <memory>

using namespace std;

//Các loại Token được sử dụng trong PL/0
typedef enum {
	NONE=0, IDENT, NUMBER,
	BEGIN, CALL, CONST, DO,  ELSE, END, FOR, IF, ODD,
	PROCEDURE, PROGRAM, THEN, TO, VAR, WHILE,PLUS, MINUS,
	TIMES, SLASH, EQU, NEQ, LSS, LEQ, GTR, GEQ, LPARENT,
	RPARENT, LBRACK, RBRACK, PERIOD, COMMA, SEMICOLON,  ASSIGN, PERCENT
} TokenType;

typedef int NumberType;

class Flag {
    public:
    int numError = 0;
    bool printError = true;
};

class LexicalAnalyzer {
private:
    const unsigned char ENDSTATE = 255;
    unsigned char state = 0;
    const int MAX_IDENT_LEN = 10;
    const int MAX_NUMBER_LEN = 6;
    bool eofFlag = false;
    bool noReadMore = false;
    vector<char> ide;
    vector<char> num;
    TokenType token;
    FILE *f = NULL;
    long line = 0;
    char c;

    unsigned char getNextState0();
    unsigned char getNextState1();
    unsigned char getNextState2();
    unsigned char getNextState3();
    unsigned char getNextState4();
    unsigned char getNextState5();

    unsigned char(LexicalAnalyzer::*nextState[6])() = {
        &LexicalAnalyzer::getNextState0, &LexicalAnalyzer::getNextState1,
        &LexicalAnalyzer::getNextState2, &LexicalAnalyzer::getNextState3,
        &LexicalAnalyzer::getNextState4, &LexicalAnalyzer::getNextState5
    };

    void error(const char* msg);
    void reset(void);
    bool getCh(void);

    void next() {
        state = (this->*nextState[state])();
    }
public:
    TokenType getNextToken();
    bool hasNextToken();
    long getLine(void);
    string* getIdent();
    NumberType getNumber();
    void setFile(FILE *f);
};

#endif // LEXICAL_ANALYZER
