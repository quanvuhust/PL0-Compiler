#ifndef SYNTAX_ANALYZER
#define SYNTAX_ANALYZER 0

#include "LexicalAnalyzer.h"

class SyntaxAnalyzer{
private:
    LexicalAnalyzer lexicalAnalyzer;
    TokenType token;

    void error (int errorCode);
    void nextToken(void);
    void factor(void);
    void term(void);
    void expression(void);
    void condition(void);
    void statement(void);
    void block(void);
public:
    SyntaxAnalyzer(LexicalAnalyzer lex):lexicalAnalyzer(lex) {}
    void program(void);

};

#endif // SYNTAX_ANALYZER
