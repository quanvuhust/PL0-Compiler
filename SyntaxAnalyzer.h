#ifndef SYNTAX_ANALYZER
#define SYNTAX_ANALYZER 0

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "SemanticAnalyzer.h"


class SyntaxAnalyzer{
private:
    LexicalAnalyzer lexicalAnalyzer;
    SemanticAnalyzer semanticAnalyzer;
    TokenType token;
    vector<SymbolTable*> tblptr;
    SymbolTable *table = nullptr;

    void declareVariable();
    void declareConst();
    void declareProc();

    void callProc();
    void assignVariable();
    void checkFactor();
    void loadOldTable();

    void error (int errorCode, string name="");
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
