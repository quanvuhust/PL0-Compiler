#ifndef PARSER_H
#define PARSER_H 1

#include "Parser.h"

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "SemanticAnalyzer.h"

#include "Library.h"
#include "CodeGenerator.h"



class Parser{
private:
    LexicalAnalyzer lexicalAnalyzer;
    SemanticAnalyzer semanticAnalyzer;
    SyntaxAnalyzer syntaxAnalyzer;
    CodeGenerator codeGenerator;
    TokenType token;
    vector<SymbolTable*> tblptr;
    int base = 0;
    int offset = 0;
    string name;

    int checkFileName(char* fileName);
    void syntaxError(int errorCode, string name="");
    void semanticError(int errorCode, string name="");

    SymbolTable *table = nullptr;
    Builtin builtin;

    void declareVariable();
    void declareConst();
    void declareProc();

    void callProc();
    void assignVariable();
    void checkFactor();
    void loadOldTable();

    void nextToken(void);
    void factor(void);
    void term(void);
    void expression(void);
    void condition(void);
    void statement(void);
    void block(void);
    void program(void);
public:
    void parse(char* fileName);
};

#endif // PARSER_H
