#include <iostream>
#include <unordered_map>
#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"

using namespace std;

Flag flag;

int main(int argc, char* argv[]) {
    FILE *input = NULL;
    /*
    if(argc < 2) {
        printf("Missing argument.\n");
        getchar();
        return 1;
    }*/
    /*
    input = fopen("./test.txt", "a+t");
    fseek(input, SEEK_END, 0);
    char endCharacter = fgetc(input);
    if(endCharacter != '\n' && endCharacter != '\r') {
        fputc('\n', input);
    }
    fclose(input);
    */
    input = fopen("./test.txt", "rt");
    unordered_map<int, const char*> ma = {
	{0, "NONE"}, {1, "IDENT"}, {2, "NUMBER"},
	{3, "BEGIN"}, {4, "CALL"}, {5, "CONST"}, {6, "DO"},  {7, "ELSE"}, {8, "END"}, {9, "FOR"}, {10, "IF"}, {11, "ODD"},
	{12, "PROCEDURE"}, {13, "PROGRAM"}, {14, "THEN"}, {15, "TO"}, {16, "VAR"}, {17, "WHILE"},{18, "PLUS"}, {19, "MINUS"},
	{20, "TIMES"}, {21, "SLASH"}, {22, "EQU"}, {23, "NEQ"}, {24, "LSS"}, {25, "LEQ"}, {26, "GTR"}, {27, "GEQ"}, {28, "LPARENT"},
	{29, "RPARENT"}, {30, "LBRACK"}, {31, "RBRACK"}, {32, "PERIOD"}, {33, "COMMA"}, {34, "SEMICOLON"},  {35, "ASSIGN"}, {36, "PERCENT"}
    };
    if(input == NULL) {
        cout << "File is not exists." << endl;
        return 1;
    }
    LexicalAnalyzer lexicalAnalyzer(input);
    /*
    while(lexicalAnalyzer.hasNextToken()) {
        cout << ma[lexicalAnalyzer.getNextToken()] << '|';
    }*/
    SyntaxAnalyzer syntaxAnalyzer(lexicalAnalyzer);
    syntaxAnalyzer.program();

    fclose(input);
    return 0;
}
